import Include
from src.fileUtils.jsonConfig import *
from src.utils.timeString import *

if __name__ == '__main__':
    taskConfig = TaskConfig(
        cid=123456,
        title="Heng_Xin",
        lastFetchTime=1689000000,
        range=(1688900000, 1689000000),
        currentTime=1689000000,
        totalDanmaku=50000,
        advancedDanmaku=2000,
        status=FetchStatus.FetchingHistory
    )

    jc = TaskConfigManager(Path("./data/task.json"))
    jc.save(taskConfig)
    print(jc.load())
    print(jc.load() == taskConfig)

    print(TimeString.strToTimestamp("1970-01-01"))
    print(TimeString.timestampToStr(0))

    ###################################################

    mainConfig = MainConfig(["123", "456"])

    jc = MainConfigManager(Path("./data/main.json"))
    jc.save(mainConfig)

    print(jc.load())
    print(jc.load() == mainConfig)