import os
import sys
import platform

APP_NAME: str = 'BiLiBiLi_DanMu_Crawling'

def getAppBaseDir(kind: str = "config") -> str:
    """
    kind: 'config' or 'data' or 'cache'
    返回一个跨平台的用户路径, 自动创建目录
    """
    system = platform.system()
    if system == "Windows":
        base = os.environ.get("APPDATA") if kind == "config" else os.environ.get("LOCALAPPDATA", os.environ.get("APPDATA"))
    elif system == "Darwin":  # macOS
        base = os.path.expanduser("~/Library/Application Support")
    else:  # Linux/Unix
        if kind == "config":
            base = os.environ.get("XDG_CONFIG_HOME", os.path.expanduser("~/.config"))
        elif kind == "data":
            base = os.environ.get("XDG_DATA_HOME", os.path.expanduser("~/.local/share"))
        else:
            base = os.environ.get("XDG_CACHE_HOME", os.path.expanduser("~/.cache"))

    path = os.path.join(base, APP_NAME)
    os.makedirs(path, exist_ok=True)
    return path

def getRelativeOutputDir() -> str:
    """
    返回相对程序的路径
    """
    workDir = os.getcwd()  # 用户从哪个目录启动你的程序
    outputDir = os.path.join(workDir, "output")
    os.makedirs(outputDir, exist_ok=True)
    return outputDir