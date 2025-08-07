from collections import defaultdict
from enum import Enum
from dataclasses import dataclass, asdict
import os
from pathlib import Path
import re
from typing import List, Tuple
import orjson

from ..utils.timeString import TimeString
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
        configId (str): 配置文件的唯一id
        cid (int): 视频唯一标识符
        title (str): 爬取的视频标题
        lastFetchTime (int): 上次执行时间, Unix 时间戳(秒)
        range (Tuple[int, int]): 爬取时间范围(左边界, 右边界), Unix 时间戳(秒)
        currentTime (int): 当前爬取的时间点, Unix 时间戳(秒)
        totalDanmaku (int): 爬取到的弹幕总数
        advancedDanmaku (int): 爬取到的神弹幕(如高级弹幕、代码弹幕、Bas弹幕)数
        status (FetchStatus): 当前爬取状态
    """
    configId: str
    cid: int
    title: str
    lastFetchTime: int
    range: Tuple[int, int]
    currentTime: int
    totalDanmaku: int
    advancedDanmaku: int
    status: FetchStatus

    def activation(self):
        if (self.range[0] == 0):
            self.range = (TimeString.strToTimestamp("2009-01-01"), self.range[1])
        if (self.range[1] == 0):
            self.range = (self.range[0], TimeString.getLocalTimestamp())
        if (self.currentTime == 0):
            self.currentTime = self.range[1]

    def toDict(self) -> dict:
        return {
            "configId": self.configId,
            "cid": self.cid,
            "title": self.title,
            "lastFetchTime": self.lastFetchTime,
            "range": list(self.range),   # tuple转list, 保证json兼容
            "currentTime": self.currentTime,
            "totalDanmaku": self.totalDanmaku,
            "advancedDanmaku": self.advancedDanmaku,
            "status": self.status  # Enum转字符串
        }

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
                configId="",
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
            configId=raw["configId"],
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
        
        # 所有爬取任务的路径map: {configId : path} 因为 cid不唯一
        self._tasksIdPathMap: dict[str, str] = defaultdict()

        # configId 和 爬取任务 的映射
        self._configIdToTaskIdMap: dict[str, str] = defaultdict()

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
            for subDirName in os.listdir(dirPath):
                subDirPath = os.path.join(dirPath, subDirName)
                if not os.path.isdir(subDirPath):
                    continue
                for fileName in os.listdir(subDirPath):
                    # 匹配形如 {cid}_config.json 的文件
                    match = re.match(r"(\d+)_config\.json", fileName)
                    if (match):
                        # 必须存在的 configId, 否则创建的时候就不是正常创建了...
                        taksConfig = TaskConfigManager(Path(f"{subDirPath}/{fileName}")).load()
                        if (taksConfig.configId != ""):
                            self._tasksIdPathMap[taksConfig.configId] = subDirPath

    def addCidPathKV(self, configId: str, path: str) -> None:
        self._tasksIdPathMap[configId] = path

    def get(self) -> MainConfig:
        return self._config

    def reRead(self) -> None:
        self._configManager = MainConfigManager(BasePath.relativePath(f"{DATA_PATH}/main_config.json"))

    def save(self) -> None:
        self._configManager.save(self._config)