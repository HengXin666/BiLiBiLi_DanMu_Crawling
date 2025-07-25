from collections import defaultdict
from logging import Logger
import logging
from logging.handlers import RotatingFileHandler
import time
import asyncio
from typing import Awaitable, Callable, Dict, Set
from fastapi import WebSocket

from ..pojo.vo.WebSocketVo import WebSocketVo
from ..api.videoApi import *
from ..api.danMaKuApi import * 
from ..fileUtils.jsonConfig import *
from ..fileUtils.danMaKuSqlite3 import *
from ..utils.timeString import *

"""
爬取流程:
    *0. 输入爬取的日期范围 [左边界, 右边界] -> 当前日期 = 右边界

    1. 爬取 [当前日期] 的历史弹幕

    2. 解析历史弹幕中 [非保护弹幕] 的弹幕中, 最早的时间

    3. 从该日期继续爬取

    *4. 如果 [当前日期 < 左边界] 则爬取结束, 否则 goto {1.}
"""

class AllDmRequests:

    @dataclass
    class TaskData:
        # 爬取的 cid
        cid: int
        # 存放路径 如 /爬取项目/爬取标题
        path: str
        # 是否运行
        isRun: bool = True

    def __init__(self) -> None:
        # 维护一个任务池 {uuid: 任务}
        self._tasks: Dict[str, asyncio.Task] = defaultdict()

        # 维护任务运行的可操作数 如: 是否暂停
        self._taskData: Dict[str, AllDmRequests.TaskData] = defaultdict()

        # 维护一个客户端连接池, 用于返回执行进度
        # {uuid: Set{客户端ws句柄}}
        self._clients: Dict[str, Set[WebSocket]] = defaultdict()

        # 维护任务的运行时消息 {任务id : [logs...]}
        self._taskLog: Dict[str, List[str]] = defaultdict()
    
    async def delTask(self, taskId: str) -> None:
        self._tasks.pop(taskId)
        self._taskData.pop(taskId)
        # 删除客户端并且断开连接
        await self._allClientsDo(
            self._clients.pop(taskId), lambda ws: ws.close())
        
    @staticmethod
    async def _allClientsDo(clis: Set[WebSocket], cb: Callable[[WebSocket], Awaitable[None]]) -> None:
        removeList = []
        for cli in clis:
            try:
                await cb(cli)
            except Exception as e:
                print(e)
                removeList.append(cli)
        for cli in removeList:
            clis.remove(cli)

    class LogProxy:
        def __init__(self, _configId: str, cid: int, msgListRef: List[str]) -> None:
            self._configId = _configId
            self._cid = cid
            self._msgListRef = msgListRef

            path = GlobalConfig()._tasksIdPathMap[_configId]
            self._logPath = Path(f"{path}/{cid}_log.log")
            self._logger = self._initLogger()

        def _initLogger(self) -> Logger:
            logger = logging.getLogger(f"logger_{self._configId}_{self._cid}")
            logger.setLevel(logging.INFO)

            if logger.hasHandlers():
                return logger  # 避免重复添加 handler

            handler = RotatingFileHandler(
                self._logPath,
                maxBytes=5 * 1024 * 1024,
                backupCount=2,
                encoding="utf-8"
            )
            formatter = logging.Formatter(
                "[%(asctime)s] [%(levelname)s] %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S"
            )
            handler.setFormatter(formatter)
            logger.addHandler(handler)
            return logger

        def info(self, msg: str) -> None:
            self._logger.info(msg)

        def warn(self, msg: str) -> None:
            self._logger.warning(msg)

        def error(self, msg: str) -> None:
            self._logger.error(msg)

        async def log(self, msg: str, clis: Set[WebSocket]) -> None:
            """ws发送日志, 并且记录到运行时内存和日志文件

            Args:
                msg (str): 日志内容
                clis (Set[WebSocket]): 客户端
            """
            # 记录日志 RAM
            self._msgListRef.append(msg)

            # 记录日志 FILE
            self.info(msg)

            # 发送日志
            await AllDmRequests._allClientsDo(clis, lambda ws: ws.send_json(WebSocketVo.log(msg)))


    async def run(self, configId: str, taskId: str, cid: int):
        """开始爬取历史弹幕

        Args:
            taskId (str): 当前任务的uuid
            cid (int): 视频cid
            path (str): 爬取数据存放路径
        """
        print("开始", taskId, cid)
        
        path = GlobalConfig()._tasksIdPathMap[configId]
        GlobalConfig()._configIdToTaskIdMap[configId] = taskId

        # 记录协程任务数据
        self._taskData[taskId] = AllDmRequests.TaskData(
            cid=cid,
            path=path,
            isRun=True
        )

        # 初始化 api
        api = DanMaKuApi(GlobalConfig().get().cookies)

        await self._allClientsDo(
            self._clients[taskId], lambda ws: ws.send_json(
                WebSocketVo.log(f"开始任务 {taskId}")))

        # 创建 & 读取 配置
        taskConfigManager = TaskConfigManager(Path(f"{path}/{cid}_config.json"))
        taskConfig = taskConfigManager.load()
        taskConfig.status = FetchStatus.FetchingHistory

        danmakuIdStorage = DanmakuIdStorage(Path(f"{path}/{cid}_dm_id.db"))
        idSet = danmakuIdStorage.selectAllDmOnlyId()

        danmakuElemStorage = DanmakuElemStorage(Path(f"{path}/{cid}_dm_data.db"))

        # 注意, 如果首尾出现 0, 我们需要手动处理
        taskConfig.activation() # 也就是活化一下数据, 但是这样前端展示的就是具体时间了哦

        # 初始化日志
        self._taskLog[taskId] = []

        logProxy = AllDmRequests.LogProxy(configId, cid, self._taskLog[taskId])

        await self._allClientsDo(
                self._clients[taskId], lambda ws: ws.send_json(
                    WebSocketVo.msg("taskConfig", taskConfig.toDict())))

        while self._taskData[taskId].isRun:
            addDm = []          # 新增弹幕数据
            addId = set()       # 新增id
            addDmCnt = 0        # 新增弹幕数量
            addSeniorDmCnt = 0  # 新增神弹幕 (高级/代码/Bas)

            maeTime: str =  TimeString.timestampToStr(taskConfig.currentTime)

            async def getDanMaKuReq() -> Tuple[int, List[DanmakuElem]]:
                for _ in range(5):
                    try:
                        resList = api.getHistoricalDanMaKu(
                            cid=taskConfig.cid,
                            date=maeTime
                        )
                        return 0, resList
                    except:
                        await logProxy.log(f"超时，重试中: {maeTime}", self._clients[taskId])
                        await asyncio.sleep(random.uniform(
                            GlobalConfig().get().timer[0],
                            GlobalConfig().get().timer[1]))
                return -1, []

            err, resList = await getDanMaKuReq()

            if (err != 0):
                taskConfig.status = FetchStatus.BanHistory
                break

            if (len(resList) == 0):
                await logProxy.log(f"爬取: {maeTime}, 没有弹幕...", self._clients[taskId])
                taskConfig.status = FetchStatus.FetchedHistoryOk
                print("没有弹幕..., 好像完了")
                break

            for it in resList:
                if (it.id not in idSet):
                    addDm.append(it)
                    idSet.add(it.id)
                    addId.add(it.id)
                    addDmCnt += 1

                    # it.mode >= 7 是高级弹幕, 代码弹幕, Bas弹幕, 统称神弹幕
                    addSeniorDmCnt += (it.mode >= 7)

                    if (not (it.attr & 1) and taskConfig.currentTime > it.ctime):
                        taskConfig.currentTime = it.ctime

            danmakuIdStorage.insertDmOnlyId(addId)
            danmakuElemStorage.insertDanMaKu(addDm)

            taskConfig.advancedDanmaku += addSeniorDmCnt
            taskConfig.totalDanmaku += addDmCnt

            if (TimeString.timestampToStr(taskConfig.currentTime) == maeTime):
                taskConfig.currentTime -= 60 * 60 * 24

            taskConfig.lastFetchTime = TimeString.getLocalTimestamp()
            taskConfigManager.save(taskConfig)

            await logProxy.log(
                f"爬取: {maeTime} 弹幕 {       \
                    taskConfig.totalDanmaku   \
                } (+{addDmCnt}) | 神弹幕 {     \
                    taskConfig.advancedDanmaku\
                } (+{addSeniorDmCnt})", self._clients[taskId])
                
            await logProxy.log(f"接下来爬取: {TimeString.timestampToStr(taskConfig.currentTime)}",
                               self._clients[taskId])

            # 让客户端更新数据预览
            await self._allClientsDo(
                self._clients[taskId], lambda ws: ws.send_json(
                    WebSocketVo.msg("taskConfig", taskConfig.toDict())))

            # 返回前端的响应 debug
            print(f"爬取: {maeTime} 弹幕 { \
                taskConfig.totalDanmaku   \
            } (+{addDmCnt}) | 神弹幕 {   \
                taskConfig.advancedDanmaku\
                } (+{addSeniorDmCnt})")
            print(f"接下来爬取: {TimeString.timestampToStr(taskConfig.currentTime)}")

            await asyncio.sleep(random.uniform(GlobalConfig().get().timer[0], GlobalConfig().get().timer[1]))

        if (not self._taskData[taskId].isRun):
            await logProxy.log(f"已暂停", self._clients[taskId])
            taskConfig.status = FetchStatus.PauseHistory

        await self._allClientsDo(
            self._clients[taskId], lambda ws: ws.send_json(
                WebSocketVo.msg("taskConfig", taskConfig.toDict())))

        # 清理
        GlobalConfig()._configIdToTaskIdMap.pop(taskConfig.configId)
        await self.delTask(taskId)

        taskConfigManager.save(taskConfig)


    @staticmethod
    def __start(cid: int, path: str):
        """开始爬取历史弹幕

        Args:
            cid (int): 视频cid
            path (str): 爬取数据存放路径
        """
        api = DanMaKuApi(GlobalConfig().get().cookies)

        # 创建 & 读取 配置
        # 注意! 在调用此方法之前, 外部应该已经创建好 taskConfig !!!
        taskConfigManager = TaskConfigManager(BasePath.relativePath(f"{path}/{cid}_config.json"))
        taskConfig = taskConfigManager.load()
        taskConfig.status = FetchStatus.FetchingHistory

        danmakuIdStorage = DanmakuIdStorage(BasePath.relativePath(f"{path}/{cid}_dm_id.db"))
        idSet = danmakuIdStorage.selectAllDmOnlyId()

        danmakuElemStorage = DanmakuElemStorage(BasePath.relativePath(f"{path}/{cid}_dm_data.db"))

        isRun = True

        while (isRun):
            addDm = []
            addId = set()
            addDmCnt = 0
            addSeniorDmCnt = 0

            # 记录当日日期
            maeTime: str =  TimeString.timestampToStr(taskConfig.currentTime)

            def getDanMaKuReq() -> Tuple[int, List[DanmakuElem]]:
                for _ in range(5):
                    # 此次可以放一个队列, 方便接受外部指挥
                    # 如: 暂停

                    try:
                        resList = api.getHistoricalDanMaKu(
                            cid=taskConfig.cid,
                            date=maeTime
                        )
                        return 0, resList
                    except:
                        print("超时")
                        # 随机暂停
                        time.sleep(5)
                return -1, []
                
            err, resList = getDanMaKuReq()

            # 被封禁了
            if (err != 0):
                taskConfig.status = FetchStatus.BanHistory
                break

            # 爬取不到任何弹幕
            if (len(resList) == 0):
                print(f"爬取: {maeTime}, 没有弹幕...")
                taskConfig.status = FetchStatus.FetchedHistoryOk
                break

            # 存放弹幕
            for it in resList:
                if (it.id not in idSet):
                    addDm.append(it)
                    idSet.add(it.id)
                    addId.add(it.id)
                    addDmCnt += 1

                    # 统计高级弹幕
                    addSeniorDmCnt += it.mode >= 7

                    # 计算当日弹幕池最早的弹幕
                    # https://github.com/HengXin666/BiLiBiLi_DanMu_Crawling/issues/14
                    if (not (it.attr & 1)
                        and taskConfig.currentTime > it.ctime): # 不是保护弹幕 并且 日期更加新
                        taskConfig.currentTime = it.ctime
            
            # 保存爬取的数据
            danmakuIdStorage.insertDmOnlyId(addId)
            danmakuElemStorage.insertDanMaKu(addDm)

            taskConfig.advancedDanmaku += addSeniorDmCnt
            taskConfig.totalDanmaku += addDmCnt
            
            # 如果 相等, 有1种情况
            # 弹幕池满了: 因此的弹幕都是同一天发送的
            # ps: 不会出现 最早的日期相等 并且不是同一天的情况
            if (TimeString.timestampToStr(taskConfig.currentTime) == maeTime):
                # 手动把时间回退到前一天 (B站时间戳单位是 秒)
                taskConfig.currentTime -= 60 * 60 * 24

            # 保存配置状态
            taskConfig.lastFetchTime = TimeString.getLocalTimestamp()
            taskConfigManager.save(taskConfig)

            # 返回前端的响应
            print(f"爬取: {maeTime} 弹幕 { \
                taskConfig.totalDanmaku   \
            } (+{addDmCnt}) | 神弹幕 {   \
                taskConfig.advancedDanmaku\
                } (+{addSeniorDmCnt})")
            print(f"接下来爬取: {TimeString.timestampToStr(taskConfig.currentTime)}")

            # 随机暂停
            time.sleep(random.uniform(GlobalConfig().get().timer[0], GlobalConfig().get().timer[1]))

        # 如果是不run, 就是暂停
        if (not isRun):
            taskConfig.status = FetchStatus.PauseHistory
        
        # 保存爬取状态到配置文件
        taskConfigManager.save(taskConfig)
        