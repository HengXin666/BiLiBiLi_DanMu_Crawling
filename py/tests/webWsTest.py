from fastapi import FastAPI, WebSocket, WebSocketDisconnect
import asyncio
import uuid
from typing import Dict, Set
from fastapi.middleware.cors import CORSMiddleware

# 使用如下命令启动
# uvicorn webWsTest:app --reload --host 0.0.0.0 --port 8000

app = FastAPI()

# 跨域请求
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # 或者 ["http://localhost:5173"]，更安全
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

tasks: Dict[str, asyncio.Task] = {}
clients: Dict[str, Set[WebSocket]] = {}

async def backgroundTask(taskId: str):
    try:
        for i in range(100):
            await asyncio.sleep(0.1)
            for ws in list(clients.get(taskId, set())):
                try:
                    await ws.send_text(f"{i+1}%")
                except Exception:
                    clients[taskId].remove(ws)
        for ws in list(clients.get(taskId, set())):
            try:
                await ws.send_text("done")
            except Exception:
                clients[taskId].remove(ws)
    except asyncio.CancelledError:
        # 任务被取消
        pass

@app.post("/start-task")
async def start_task():
    taskId = str(uuid.uuid4())
    clients[taskId] = set()  # 一定先初始化clients字典
    task = asyncio.create_task(backgroundTask(taskId))
    tasks[taskId] = task
    return {"taskId": taskId}

@app.websocket("/ws/{taskId}")
async def websocket_endpoint(websocket: WebSocket, taskId: str):
    await websocket.accept()
    if taskId not in clients:
        clients[taskId] = set()
    clients[taskId].add(websocket)
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        clients[taskId].remove(websocket)
