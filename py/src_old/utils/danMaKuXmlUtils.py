import os

from .baseDirUtils import getRelativeOutputDir

# 输出目录
outputDir = getRelativeOutputDir()

class DanMaKuXmlUtils:
    def initXmlHead(fileName:str, cid: int) -> bool:
        """
        创建xml文件头, 返回是否创建成功(True: 成功; False: 失败)
        """
        try:
            if not os.path.isdir(outputDir):
                os.makedirs(outputDir)
            filePath = os.path.join(outputDir, fileName)
            with open(filePath, 'x', encoding='UTF-8') as f: # 文件已存在则, 报错
                f.write(f'<?xml version="1.0" encoding="UTF-8"?><i><chatserver>chat.bilibili.com</chatserver><chatid>{cid}</chatid><mission>0</mission><maxlimit>3000</maxlimit><state>0</state><real_name>0</real_name><source>k-v</source>\n')
            return True
        except:
            return False
    
    def remove(fileName: str):
        os.remove(os.path.join(outputDir, fileName))

    def writeDmToXml(fileName: str, xmlDmList: list):
        """
        写入列表内容到文件
        """
        filePath = os.path.join(outputDir, fileName)
        with open(filePath, 'a', encoding='UTF-8') as f:
            for dm in xmlDmList:
                f.write(dm)

    def writeDmEndToXml(fileName: str):
        """
        为弹幕文件写入终止终止符 </i>
        """
        filePath = os.path.join(outputDir, fileName)
        with open(filePath, 'a', encoding='UTF-8') as f:
            f.write("</i>")

    def isLineCountGreaterThan(fileName: str, n: int = 3000) -> int:
        """
        判断文件函数是否大于 n, 否则返回实际行数

        :param file_name: 文件名
        :param n: 行数阈值, 默认为 3000
        :return: 文件的实际行数
        """
        filePath = os.path.join(outputDir, fileName)
        nowLineCnt = 0
        with open(filePath, "rb") as f:
            for _ in f:
                nowLineCnt += 1 
                if nowLineCnt >= n:
                    return nowLineCnt - 1
        return nowLineCnt - 1

    def readLastNLines(fileName: str, n = 3000, block = -1024) -> list[bytes]:
        """
        读取文件末尾的前n行
        """
        filePath = os.path.join(outputDir, fileName)
        with open(filePath, "rb") as f:
            f.seek(0, 2)
            filesize = f.tell()
            while True:
                if filesize >= abs(block):
                    f.seek(block, 2)
                    s = f.readlines()
                    if len(s) > n:
                        return s[-n:]
                    else:
                        block <<= 1
                else:
                    block = -filesize

    def formatDanmaku(it: tuple) -> str:
        """序列化弹幕为xml

        Args:
            it (tuple): 弹幕项

        Returns:
            str: xmlStr
        """
        pBase = f"{it[0]},{it[1]},{it[2]},{it[3]},{it[4]},{it[5]},{it[6]},{it[7]}"
        # 注意 8 是可选字段, 合法范围是 [1, 10], 0 表示该字段不存在
        pFull = f"{pBase},{it[8]}" if it[8] != 0 else pBase
        return f'<d p="{pFull}">{it[9]}</d>\n'