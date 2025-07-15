# -*- coding: utf-8 -*-
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import uvicorn

from src.controller.allDmReqController import allDmReqController
from src.controller.mainConfigController import mainConfigController
from src.controller.videoInfoController import videoInfoController

app = FastAPI()

app.include_router(allDmReqController)
app.include_router(mainConfigController)
app.include_router(videoInfoController)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

if __name__ == '__main__':
    uvicorn.run("main:app", host="0.0.0.0", port=28299, reload=True)