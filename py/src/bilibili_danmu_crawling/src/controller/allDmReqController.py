import asyncio
from dataclasses import dataclass
import io
from pathlib import Path
import uuid
from typing import List, Optional, Tuple
from fastapi import APIRouter, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.responses import StreamingResponse
from pydantic import BaseModel
import urllib

from ..pojo.vo.WebSocketVo import WebSocketVo
from ..pojo.vo.AllTaskDataVo import AllTaskDataVo
from ..utils.basePath import BasePath
from ..utils.timeString import TimeString
from ..fileUtils.dirUtils import deleteFolder, deleteIfParentEmpty
from ..fileUtils.jsonConfig import DATA_PATH, GlobalConfig, TaskConfig, TaskConfigManager
from ..fileUtils.danMaKuXml import DanMaKuXml
from ..api.videoApi import VideoPart
from ..tasks.AllDmRequests import AllDmRequests
from ..pojo.vo.ResponseModel import ResponseModel
from ..fileUtils.allTaskData import getAllTaskData as _getAllTaskData

allDmReqController = APIRouter(prefix="/allDm")
allDmReqManager = AllDmRequests()

class StartTaskVo(BaseModel):
    cid: int
    configId: str

class VidoPartConfigVo(BaseModel):
    path: str
    data: VideoPart
    range: Tuple[int, int] # 爬取范围时间戳
                           # 0 表示默认, 即 [0, 0] 表示 从当前爬取到不能爬取

class ExportXmlOptions(BaseModel):
    configId: str
    cid: int
    fileName: str # 仅文件名称, 我们内部需要强制加上.xml
    includeWeight: bool # 是否导出弹幕权重

class DeleteTasksVo(BaseModel):
    configIds: List[str]

@allDmReqController.post("/getAllRuningTask", response_model=ResponseModel[dict[int, str]])
def getAllRuningTask():
    """获取所有正在运行的任务Id (TaskId)

    Returns:
        _type_: {"配置文件Id": "任务Id"}
    """
    return ResponseModel.success(GlobalConfig()._configIdToTaskIdMap)

@allDmReqController.post("/startTask", response_model=ResponseModel[dict])
async def startTask(startTask: StartTaskVo):
    """启动爬虫任务, 任务直接在后台运行

    Args:
        startTask (StartTaskVo): 启动任务标识 (cid, taskId)

    Returns:
        _type_: TaskId 运行中的任务Id
    """
    taskId = str(uuid.uuid4())
    allDmReqManager._clients[taskId] = set() # 先初始化客户端池
    task = asyncio.create_task(allDmReqManager.run(startTask.configId, taskId, startTask.cid))
    allDmReqManager._tasks[taskId] = task
    return ResponseModel.success({
        "taskId": taskId
    })

@allDmReqController.post("/stopTask/{taskId}", response_model=ResponseModel[None])
async def stopTask(taskId: str):
    """停止任务

    Args:
        taskId (str): 任务Id

    Returns:
        _type_: ok
    """
    try:
        allDmReqManager._taskData[taskId].isRun = False
        return ResponseModel.success()
    except:
        return ResponseModel.error(msg="taskId does not exist")

@allDmReqController.websocket("/ws/{taskId}")
async def taskState(ws: WebSocket, taskId: str):
    """webSocket 消息

    Args:
        ws (WebSocket): _description_
        taskId (str): 任务Id
    """
    await ws.accept()
    
    # 全部推送, 因为新连接用户肯定没有数据
    # @todo 又因为前端只能通过刷新来触发ws重连, 故 ok
    for msg in allDmReqManager._taskLog[taskId]:
        await ws.send_json(WebSocketVo.log(msg))
    
    allDmReqManager._clients[taskId].add(ws)
    try:
        await ws.send_json(WebSocketVo.log("已连接到后端"))
        while True:
            await asyncio.sleep(60 * 60) # 保持连接, 心跳可选
    except WebSocketDisconnect:
        allDmReqManager._clients[taskId].remove(ws)

@allDmReqController.get("/getTaskConfig/{configId}", response_model=ResponseModel[TaskConfig])
def getTaskConfig(configId: str, cid: Optional[str] = None):
    """获取任务配置

    Args:
        configId (str): 配置文件Id
        cid (Optional[str], optional): 视频cid, 不可为空. Defaults to None.

    Returns:
        _type_: _description_
    """
    if (cid == None):
        return ResponseModel.error(msg=f"应该传入 cid 查询参数")
    try:
        path = GlobalConfig()._tasksIdPathMap[configId]
        return ResponseModel.success(
            TaskConfigManager(Path(f"{path}/{cid}_config.json")).load()
        )
    except:
        return ResponseModel.error(msg=f"该 cid = {cid} 的任务配置文件不存在")

@allDmReqController.post("/setTaskConfig", response_model=ResponseModel[None])
def setTaskConfig(config: TaskConfig):
    """设置配置 (更新配置)

    Args:
        config (TaskConfig): 新配置

    Returns:
        _type_: ok
    """
    try:
        path = GlobalConfig()._tasksIdPathMap[config.configId]
        taskConfigManager = TaskConfigManager(Path(
            f"{path}/{config.cid}_config.json"))
        taskConfigManager.save(config)
        return ResponseModel.success()
    except:
        return ResponseModel.error()
    
@allDmReqController.post("/deleteTasks", response_model=ResponseModel[None])
def deleteTasks(delList: DeleteTasksVo):
    """删除配置, 如果该父目录只有这一个子文件夹, 则连同父目录一起删除

    Args:
        delList (DeleteTasksVo): 配置Id 列表

    Returns:
        _type_: ok
    """
    try:
        for configId in delList.configIds:
            path = Path(GlobalConfig()._tasksIdPathMap[configId])
            deleteFolder(path)
            deleteIfParentEmpty(path)
        return ResponseModel.success()
    except:
        return ResponseModel.error()

@allDmReqController.post("/initTaskConfig", response_model=ResponseModel[None])
def initTaskConfig(config: VidoPartConfigVo):
    """初始化任务, 创建对应的文件

    Args:
        config (VidoPartConfigVo): 配置
    """
    try:
        taskConfigManager = TaskConfigManager(BasePath.relativePath(
            f"{DATA_PATH}/{config.path}/{config.data.cid}_config.json"))
        taskConfig = taskConfigManager.load()
        taskConfig.configId = f"{uuid.uuid4()}-{config.data.cid}" # 如果这样都能重复, 那也是神人操作了
        taskConfig.cid = config.data.cid
        taskConfig.title = config.data.part
        taskConfig.range = config.range
        taskConfig.lastFetchTime = TimeString.getLocalTimestamp()
        taskConfigManager.save(taskConfig)
        # 这里需测试跨平台情况 (路径)
        GlobalConfig().addCidPathKV(taskConfig.configId, str(BasePath.relativePath(f"{DATA_PATH}/{config.path}")))
        return ResponseModel.success()
    except:
        return ResponseModel.error()

@allDmReqController.get("/getAllTaskData", response_model=ResponseModel[List[AllTaskDataVo]])
def getAllTaskData():
    """获取所有任务数据

    Returns:
        _type_: (标题 + 配置列表) 的列表
    """
    return ResponseModel.success(_getAllTaskData())

@allDmReqController.post("/exportXml")
def exportXml(options: ExportXmlOptions):
    """导出Xml (时间可能比较久, 是否可以优化?)

    Args:
        options (ExportXmlOptions): 导出选项

    Returns:
        _type_: .xml 下载
    """
    file_name = options.fileName
    if not file_name.endswith(".xml"):
        file_name += ".xml"
    try:
        path = GlobalConfig()._tasksIdPathMap.get(options.configId)
        xmlStr = DanMaKuXml.exportXml(Path(f"{path}/{options.cid}_dm_data.db"), options.cid, options.includeWeight)
    except:
        return HTTPException(status_code=404, detail="路径不存在, cid错误")
    
    encodedFilename = urllib.parse.quote(file_name, encoding='utf-8') # type: ignore
    contentDisposition = f"attachment; filename*=UTF-8''{encodedFilename}"
    
    return StreamingResponse(
        io.BytesIO(xmlStr.encode("utf-8")),
        media_type="application/xml",
        headers={"Content-Disposition": contentDisposition}
    )
    