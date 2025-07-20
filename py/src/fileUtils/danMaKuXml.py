from pathlib import Path
from xml.sax.saxutils import escape

from .danMaKuSqlite3 import DanmakuElemStorage

class DanMaKuXml:
    @staticmethod
    def exportXml(path: Path, cid: int, includeWeight: bool):
        db = DanmakuElemStorage(path)
        danMaKuList = db.selectAllDanMaKu()

        # 组装弹幕
        xmlLines = [
            '<?xml version="1.0" encoding="UTF-8"?>',
            '<i>',
            '    <chatserver>chat.bilibili.com</chatserver>',
           f'    <chatid>{cid}</chatid>',
            '    <mission>0</mission>',
            '    <maxlimit>1500</maxlimit>',
            '    <state>0</state>',
            '    <real_name>0</real_name>',
            '    <source>e-r</source>'
        ]

        # 判断是否导出权重, if 提到外面, 提高性能
        if (includeWeight):
            for dm in danMaKuList:
                # 转成秒 (最多保留5位小数)
                appearTime = round(dm.progress / 1000.0, 5)

                # 弹幕属性
                pAttrs = [
                    str(appearTime),    # 00 出现时间
                    str(dm.mode),       # 01 弹幕类型
                    str(dm.fontsize),   # 02 弹幕字号
                    str(dm.color),      # 03 弹幕颜色
                    str(dm.ctime),      # 04 弹幕发送时间
                    str(dm.pool),       # 05 弹幕池类型
                    dm.midHash,         # 06 发送者mid的HASH
                    dm.idStr            # 07 弹幕dmid
                ]

                # 弹幕的屏蔽等级
                if dm.weight != 0:
                    pAttrs.append(str(dm.weight))

                # 转义内容, 默认 仅转义 & < >
                content = escape(dm.content)

                # 生成 <d> 标签
                xmlLines.append(f'    <d p="{",".join(pAttrs)}">{content}</d>')
        else:
            for dm in danMaKuList:
                # 转成秒 (最多保留5位小数)
                appearTime = round(dm.progress / 1000.0, 5)

                # 弹幕属性
                pAttrs = [
                    str(appearTime),    # 00 出现时间
                    str(dm.mode),       # 01 弹幕类型
                    str(dm.fontsize),   # 02 弹幕字号
                    str(dm.color),      # 03 弹幕颜色
                    str(dm.ctime),      # 04 弹幕发送时间
                    str(dm.pool),       # 05 弹幕池类型
                    dm.midHash,         # 06 发送者mid的HASH
                    dm.idStr            # 07 弹幕dmid
                ]

                # 转义内容, 默认 仅转义 & < >
                content = escape(dm.content)

                # 生成 <d> 标签
                xmlLines.append(f'    <d p="{",".join(pAttrs)}">{content}</d>')

        xmlLines.append('</i>')

        return "\n".join(xmlLines)