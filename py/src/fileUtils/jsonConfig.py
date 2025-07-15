from enum import Enum
from dataclasses import dataclass, asdict
import os
from pathlib import Path
import re
from typing import List, Tuple
import orjson

from ..utils.singletonTemplate import Singleton
from ..utils.basePath import BasePath

class FetchStatus(str, Enum):
    """爬取状态, 标识当前弹幕爬取任务的进度和状态"""
    FetchingHistory = "爬取历史弹幕"
    PauseHistory = "暂停爬取历史弹幕"
    FetchedHistoryOk = "爬取历史弹幕完成"
    BanHistory = "爬取历史弹幕被封禁中"
    FetchingRealtime = "正在爬取实时弹幕"
    PauseRealtime = "暂停爬取实时弹幕"
    BanRealtime = "实时弹幕被封禁中"

@dataclass
class TaskConfig:
    """
    弹幕爬取任务配置数据类
    
    Attributes:
        cid (int): 视频唯一标识符
        title (str): 爬取的视频标题
        lastFetchTime (int): 上次执行时间, Unix 时间戳(秒)
        range (Tuple[int, int]): 爬取时间范围(左边界, 右边界), Unix 时间戳(秒)
        currentTime (int): 当前爬取的时间点, Unix 时间戳(秒)
        totalDanmaku (int): 爬取到的弹幕总数
        advancedDanmaku (int): 爬取到的高级弹幕(如代码弹幕、Bas弹幕)数
        status (FetchStatus): 当前爬取状态
    """
    cid: int
    title: str
    lastFetchTime: int
    range: Tuple[int, int]
    currentTime: int
    totalDanmaku: int
    advancedDanmaku: int
    status: FetchStatus

class TaskConfigManager:
    """
    任务配置管理器, 负责读取与保存任务配置到 JSON 文件
    
    参数:
        path (Path): 配置文件路径, 支持自动创建父目录
    """
    def __init__(self, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True) # 自动创建配置文件所在目录
        self.path = path
        # 文件不存在时保存默认配置, 避免后续读取报错
        if not self.path.exists():
            self.save(TaskConfig(
                cid=0,
                title="",
                lastFetchTime=0,
                range=(0, 0),
                currentTime=0,
                totalDanmaku=0,
                advancedDanmaku=0,
                status=FetchStatus.PauseHistory
            ))

    def load(self) -> TaskConfig:
        """读取配置文件, 反序列化为 TaskConfig 对象"""
        raw = orjson.loads(self.path.read_bytes())
        return TaskConfig(
            cid=raw["cid"],
            title=raw["title"],
            lastFetchTime=raw["lastFetchTime"],
            range=tuple(raw["range"]),
            currentTime=raw["currentTime"],
            totalDanmaku=raw["totalDanmaku"],
            advancedDanmaku=raw["advancedDanmaku"],
            status=FetchStatus(raw["status"])
        )
    
    def save(self, config: TaskConfig):
        """将 TaskConfig 对象序列化写入配置文件"""
        data = asdict(config)
        # 枚举类型需要存储它的字符串值
        data["status"] = config.status.value
        self.path.write_bytes(orjson.dumps(data, option=orjson.OPT_INDENT_2))

@dataclass
class MainConfig:
    """
    主配置数据类
    
    Attributes:
        cookies (List[str]): 凭证列表
    """
    cookies: List[str]
    # 爬取间隔 [L, R]
    timer: Tuple[int, int]

class MainConfigManager:
    def __init__(self, path: Path):
        path.parent.mkdir(parents=True, exist_ok=True)
        self.path = path
        if not self.path.exists():
            self.save(MainConfig(
                cookies=[],
                timer=(6, 8)
            ))

    def load(self) -> MainConfig:
        """读取配置文件, 反序列化为 MainConfig 对象"""
        raw = orjson.loads(self.path.read_bytes())
        return MainConfig(
            cookies=raw["cookies"],
            timer=raw["timer"]
        )
    
    def save(self, config: MainConfig):
        """将 MainConfig 对象序列化写入配置文件"""
        data = asdict(config)
        self.path.write_bytes(orjson.dumps(data, option=orjson.OPT_INDENT_2))

DATA_PATH = "reqData"

@Singleton
class GlobalConfig:
    def __init__(self) -> None:
        self._configManager = MainConfigManager(BasePath.relativePath(f"{DATA_PATH}/main_config.json"))
        self._config: MainConfig = self._configManager.load()
        
        # 所有爬取任务的路径map: {cid : path}
        self._tasksPathMap: dict[int, str] = dict()

        self._initTasksPathMap()

    def _initTasksPathMap(self) -> None:
        rootPath = BasePath.relativePath(f"{DATA_PATH}/")
        # 遍历 DATA_PATH 下的文件夹
        # 的文件夹, 里面有 ${path}/${cid}_config.json
        # 加入 self._tasksPathMap {cid : path}
        for dirName in os.listdir(rootPath):
            dirPath = os.path.join(rootPath, dirName)
            if not os.path.isdir(dirPath):
                continue

            for fileName in os.listdir(dirPath):
                # 匹配形如 {cid}_config.json 的文件
                match = re.match(r"(\d+)_config\.json", fileName)
                if match:
                    cid = int(match.group(1))
                    self._tasksPathMap[cid] = dirPath

    def addCidPathKV(self, cid: int, path: str) -> None:
        self._tasksPathMap[cid] = path

    def get(self) -> MainConfig:
        return self._config

    def reRead(self) -> None:
        self._configManager = MainConfigManager(BasePath.relativePath(f"{DATA_PATH}/main_config.json"))

    def save(self) -> None:
        self._configManager.save(self._config)