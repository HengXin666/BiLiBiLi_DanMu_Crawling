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
    def start():
        cookies = ["f45db2c1%2C1755421234%2Cbc613%2A21CjCFD5eVfJiSSWn8O1FE9CR6lOcA1NKxzBJS7b5VO51ye-etSuITLEU8t13fX2lmhTESVlpRcjdhUElRQjdEUk40SzNaUmlHdXR0bkp2QTdZNVJPTU5abUhQbnp1VnpCOHhHdkNSbUFBN29jVDkzekVDX2cwd3VsMHFCQjdZeXFKSFlFZ1Fsc1ZBIIEC"]
        api = DanMaKuApi(cookies)
        err, cidList = VideoApi.getCidPart("https://www.bilibili.com/video/BV1Zx7Kz6EZP/?spm_id_from=333.337.search-card.all.click&vd_source=67ebe8ad7f6fc7a37e0608d544169bd8")
        if (err != 0):
            print("err:", err)
            return
        print(cidList) # @debug
        taskConfig = TaskConfig(
            cid=cidList[0].cid,
            title=cidList[0].part,
            lastFetchTime=TimeString.strToTimestamp(TimeString.getLocalTimeStr()),
            range=(
                TimeString.strToTimestamp("2020-01-02"),
                TimeString.strToTimestamp(TimeString.getLocalTimeStr())),
            currentTime=TimeString.strToTimestamp(TimeString.getLocalTimeStr()),
            totalDanmaku=0,
            advancedDanmaku=0,
            status=FetchStatus.FetchingHistory
        )

        idSet = set()
        onlyDm = []
        isRun = True
        exitTryCnt = 0

        while (isRun):
            addDmCnt = 0
            addSeniorDmCnt = 0
            maeTime: str =  TimeString.timestampToStr(taskConfig.currentTime)
            resList = api.getHistoricalDanMaKu(
                cid=taskConfig.cid,
                date=maeTime
            )

            if (len(resList) == 0):
                print(f"爬取: {maeTime}, 没有弹幕...")
                break

            for it in resList:
                if (it.id not in idSet):
                    idSet.add(it.id)
                    onlyDm.append(it)
                    addDmCnt += 1

                    # 统计高级弹幕
                    addSeniorDmCnt += it.mode >= 7

                    if (not (it.attr & 1)
                        and taskConfig.currentTime > it.ctime): # 不是保护弹幕 并且 日期更加新
                        taskConfig.currentTime = it.ctime

            taskConfig.advancedDanmaku += addSeniorDmCnt
            taskConfig.totalDanmaku += addDmCnt
            
            if (TimeString.timestampToStr(taskConfig.currentTime) != maeTime):
                exitTryCnt = 0
            else:
                taskConfig.currentTime -= 60 * 60 * 24
                if (addDmCnt != 0):
                    exitTryCnt = 0
                else:
                    exitTryCnt += 1

            isRun = exitTryCnt <= 5

            print(f"爬取: {maeTime} 弹幕 { \
                taskConfig.totalDanmaku   \
            } (+{addDmCnt}) | 高级弹幕 {   \
                taskConfig.advancedDanmaku\
                } (+{addSeniorDmCnt})")
            print(f"接下来爬取: {TimeString.timestampToStr(taskConfig.currentTime)}")
            # @todo 还有情况就是, 很多天弹幕都是一样的, 可能需要手动向前探索

            # 随机暂停
            time.sleep(5)


        