import sqlite3
from pathlib import Path
from typing import List

from ..api.danMaKuApi import DanmakuElem, DmColorfulType

class DanmakuElemStorage:
    """弹幕元数据 db"""
    def __init__(self, dbPath: Path):
        dbPath.parent.mkdir(parents=True, exist_ok=True)  # 自动创建文件夹
        self.conn = sqlite3.connect(dbPath)
        self._create_table()
    
    def _create_table(self):
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS danmaku (
                id INTEGER PRIMARY KEY,
                progress INTEGER NOT NULL,
                mode INTEGER NOT NULL,
                fontsize INTEGER NOT NULL,
                color INTEGER NOT NULL,
                mid_hash TEXT NOT NULL,
                content TEXT NOT NULL,
                ctime INTEGER NOT NULL,
                action TEXT NOT NULL,
                pool INTEGER NOT NULL,
                id_str TEXT NOT NULL,
                attr INTEGER NOT NULL,
                weight INTEGER NOT NULL,
                animation TEXT,
                colorful INTEGER
            )
        ''')
        # 为 ctime 建立索引, 方便在找 max ctime 的时候查询的时间复杂度为 O(1) or O(logn)
        self.conn.execute('CREATE INDEX IF NOT EXISTS idx_danmaku_ctime ON danmaku(ctime)')
        self.conn.commit()

    def insertDanMaKu(self, elems: List[DanmakuElem]):
        """插入弹幕

        Args:
            elems (List[DanmakuElem]): 弹幕元数据列表
        """
        values = []
        for e in elems:
            values.append((
                e.id,
                e.progress,
                e.mode,
                e.fontsize,
                e.color,
                e.midHash,
                e.content,
                e.ctime,
                e.action,
                e.pool,
                e.idStr,
                e.attr,
                e.weight,
                e.animation,
                e.colorful.value if e.colorful else None
            ))
        self.conn.executemany('''
            INSERT OR IGNORE INTO danmaku 
            (id, progress, mode, fontsize, color, mid_hash, content, ctime, action, pool, id_str, attr, weight, animation, colorful)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', values)
        self.conn.commit()

    def selectAllDanMaKu(self) -> List[DanmakuElem]:
        """查询所有的弹幕

        Returns:
            List[DanmakuElem]: 弹幕元数据列表
        """
        cursor = self.conn.execute('SELECT * FROM danmaku')
        result = []
        for row in cursor:
            result.append(DanmakuElem(
                id=row[0],
                progress=row[1],
                mode=row[2],
                fontsize=row[3],
                color=row[4],
                midHash=row[5],
                content=row[6],
                ctime=row[7],
                action=row[8],
                pool=row[9],
                idStr=row[10],
                attr=row[11],
                weight=row[12],
                animation=row[13],
                colorful=DmColorfulType(row[14]) if row[14] is not None else None
            ))
        return result
    
    def selectLatestCtime(self) -> int:
        """查询最大的 ctime (最新弹幕时间戳)

        Returns:
            int: 最大的 ctime 值
        """
        cursor = self.conn.execute('SELECT MAX(ctime) FROM danmaku')
        row = cursor.fetchone()
        return row[0] if row and row[0] is not None else 0

class DanmakuIdStorage:
    """存储弹幕唯一id 要求: 传入的是已经去重的id """
    def __init__(self, dbPath: Path):
        dbPath.parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(dbPath)
        self._create_table()
    
    def _create_table(self):
        self.conn.execute('''
            CREATE TABLE IF NOT EXISTS danmaku_ids (
                id INTEGER PRIMARY KEY
            )
        ''')
        self.conn.commit()
    
    def insertDmOnlyId(self, ids: set[int]):
        self.conn.executemany(
            'INSERT OR IGNORE INTO danmaku_ids (id) VALUES (?)',
            ((mid,) for mid in ids)
        )
        self.conn.commit()
    
    def selectAllDmOnlyId(self) -> set[int]:
        return {row[0] for row in self.conn.execute('SELECT id FROM danmaku_ids')}