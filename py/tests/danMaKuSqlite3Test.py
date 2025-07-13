from pathlib import Path

import Include
from src.api.danMaKuApi import DanmakuElem, DmColorfulType
from src.fileUtils.danMaKuSqlite3 import DanmakuElemStorage, DanmakuIdStorage

if __name__ == '__main__':
    storage = DanmakuElemStorage(Path("./data/danmaku.db"))
    
    test_data = [
        DanmakuElem(
            id=3, progress=1000, mode=1, fontsize=25, color=0xffffff,
            midHash="abc123", content="测试弹幕", ctime=1620000000,
            action="", pool=0, idStr="1", attr=0, weight=1,
            animation=None, colorful=DmColorfulType.NoneType
        ),
        DanmakuElem(
            id=4, progress=2000, mode=1, fontsize=25, color=0xff0000,
            midHash="def456", content="测试弹幕2", ctime=1620000010,
            action="", pool=0, idStr="2", attr=0, weight=1,
            animation="fade", colorful=DmColorfulType.VipGradualColor
        ),
    ]
    
    storage.insertDanMaKu(test_data)
    
    elems = storage.selectAllDanMaKu()
    for e in elems:
        print(e)

    ######################################################################

    storage = DanmakuIdStorage(Path("./data/mid_store.db"))
    
    # 示例输入
    mids = {1001, 1002, 1003}
    
    storage.insertDmOnlyId(mids)
    
    all_mids = storage.selectAllDmOnlyId()
    print(all_mids)
