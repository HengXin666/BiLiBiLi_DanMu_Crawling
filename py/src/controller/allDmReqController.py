import asyncio
import uuid
from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from pydantic import BaseModel

from ..tasks.AllDmRequests import AllDmRequests
from ..pojo.vo.ResponseModel import ResponseModel

allDmReqController = APIRouter(prefix="/allDm")
allDmReqManager = AllDmRequests()

class StartTask(BaseModel):
    cid: int
    path: str

@allDmReqController.post("/startTask", response_model=ResponseModel[dict])
async def startTask(startTask: StartTask):
    taskId = str(uuid.uuid4())
    allDmReqManager._clients[taskId] = set()  # 先初始化客户端池
    task = asyncio.create_task(allDmReqManager.run(taskId, startTask.cid, startTask.path))
    allDmReqManager._tasks[taskId] = task
    return ResponseModel.success({
        "taskId": taskId
    })

@allDmReqController.post("/stopTask", response_model=ResponseModel[None])
async def stopTask(taskId: str):
    try:
        allDmReqManager._taskData[taskId].isRun = False
        return ResponseModel.success()
    except:
        return ResponseModel.error(msg="taskId does not exist")

@allDmReqController.websocket("/ws/{taskId}")
async def taskState(ws: WebSocket, taskId: str):
    await ws.accept()
    allDmReqManager._clients[taskId].add(ws)
    try:
        while True:
            await asyncio.sleep(60 * 60) # 保持连接, 心跳可选
    except WebSocketDisconnect:
        allDmReqManager._clients[taskId].remove(ws)
