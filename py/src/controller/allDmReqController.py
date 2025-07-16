import asyncio
import io
from pathlib import Path
import uuid
from typing import List, Tuple
from fastapi import APIRouter, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.responses import StreamingResponse
from pydantic import BaseModel
import urllib

from ..pojo.vo.AllTaskDataVo import AllTaskDataVo
from ..utils.basePath import BasePath
from ..utils.timeString import TimeString
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

class VidoPartConfigVo(BaseModel):
    path: str
    data: VideoPart
    range: Tuple[int, int] # 爬取范围时间戳
                           # 0 表示默认, 即 [0, 0] 表示 从当前爬取到不能爬取

class TaskIdVo(BaseModel):
    taskId: str

class ExportXmlOptions(BaseModel):
    cid: int
    fileName: str # 仅文件名称, 我们内部需要强制加上.xml
    includeWeight: bool # 是否导出弹幕权重

@allDmReqController.post("/startTask", response_model=ResponseModel[dict])
async def startTask(startTask: StartTaskVo):
    taskId = str(uuid.uuid4())
    allDmReqManager._clients[taskId] = set() # 先初始化客户端池
    task = asyncio.create_task(allDmReqManager.run(taskId, startTask.cid))
    allDmReqManager._tasks[taskId] = task
    return ResponseModel.success({
        "taskId": taskId
    })

@allDmReqController.post("/stopTask", response_model=ResponseModel[None])
async def stopTask(json: TaskIdVo):
    try:
        allDmReqManager._taskData[json.taskId].isRun = False
        return ResponseModel.success()
    except:
        return ResponseModel.error(msg="taskId does not exist")

@allDmReqController.websocket("/ws/{taskId}")
async def taskState(ws: WebSocket, taskId: str):
    await ws.accept()
    allDmReqManager._clients[taskId].add(ws)
    try:
        await ws.send_text("欢迎")
        while True:
            await asyncio.sleep(60 * 60) # 保持连接, 心跳可选
    except WebSocketDisconnect:
        allDmReqManager._clients[taskId].remove(ws)

@allDmReqController.get("/getTaskConfig/{cid}", response_model=ResponseModel[TaskConfig])
def getTaskConfig(cid: int):
    try:
        path = GlobalConfig()._tasksPathMap[cid]
        return ResponseModel.success(
            TaskConfigManager(Path(f"{path}/{cid}_config.json")).load()
        )
    except:
        return ResponseModel.error(msg=f"该 cid = {cid} 的任务配置文件不存在")

@allDmReqController.post("/setTaskConfig", response_model=ResponseModel[None])
def setTaskConfig(config: TaskConfig):
    try:
        path = GlobalConfig()._tasksPathMap[config.cid]
        taskConfigManager = TaskConfigManager(Path(
            f"{path}/{config.cid}_config.json"))
        taskConfigManager.save(config)
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
        taskConfig.cid = config.data.cid
        taskConfig.title = config.data.part
        taskConfig.range = config.range
        taskConfig.lastFetchTime = TimeString.getLocalTimestamp()
        taskConfigManager.save(taskConfig)
        GlobalConfig().addCidPathKV(taskConfig.cid, f"{DATA_PATH}/{config.path}")
        return ResponseModel.success()
    except:
        return ResponseModel.error()

@allDmReqController.get("/getAllTaskData", response_model=ResponseModel[List[AllTaskDataVo]])
def getAllTaskData():
    return ResponseModel.success(_getAllTaskData())

@allDmReqController.post("/exportXml")
def exportXml(options: ExportXmlOptions):
    file_name = options.fileName
    if not file_name.endswith(".xml"):
        file_name += ".xml"
    try:
        path = GlobalConfig()._tasksPathMap.get(options.cid)
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
    