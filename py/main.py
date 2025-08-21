# -*- coding: utf-8 -*-
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import argparse

from src.controller.allDmReqController import allDmReqController
from src.controller.basicInfoController import basicInfoController
from src.controller.mainConfigController import mainConfigController
from src.controller.videoInfoController import videoInfoController

import os, sys

# 兼容 PyInstaller
if getattr(sys, "frozen", False):
    os.chdir(sys._MEIPASS)  # type: ignore

parser = argparse.ArgumentParser(description="bilibili_danmu_crawling")

parser.add_argument(
    "-p", "--port", type=int, default=28299, help="输入服务器将要打开的端口"
)
parser.add_argument("-c", "--config", type=str, help="输入配置文件路径")
parser.add_argument("-d", "--dev", action="store_true", help="开启调试模式")

args = parser.parse_args()

print("启动参数：", args)

if args.config is not None:
    os.environ["BILIBILI_DANMU_CRAWLING_CONFIG_PATH"] = args.config

os.environ["BILIBILI_DANMU_CRAWLING_DEV"] = str(args.dev)

app = FastAPI()

app.include_router(allDmReqController)
app.include_router(basicInfoController)
app.include_router(mainConfigController)
app.include_router(videoInfoController)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=args.port)
