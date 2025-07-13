import time
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

class AllDmReq:
    @staticmethod
    def start(cid: int, path: str):
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

            print(f"爬取: {maeTime} 弹幕 { \
                taskConfig.totalDanmaku   \
            } (+{addDmCnt}) | 高级弹幕 {   \
                taskConfig.advancedDanmaku\
                } (+{addSeniorDmCnt})")
            print(f"接下来爬取: {TimeString.timestampToStr(taskConfig.currentTime)}")

            # 随机暂停
            time.sleep(5)

        # 如果是不run, 就是暂停
        if (not isRun):
            taskConfig.status = FetchStatus.PauseHistory
        
        # 保存爬取状态到配置文件
        taskConfigManager.save(taskConfig)
        