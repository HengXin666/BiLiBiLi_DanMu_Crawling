import os

# 输出目录
outputDir = os.path.join(os.path.dirname(__file__), '..', '..', 'output')

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
                f.write(f'<?xml version="1.0" encoding="UTF-8"?><i><chatserver>chat.bilibili.com</chatserver><chatid>{cid}</chatid><mission>0</mission><maxlimit>3000</maxlimit></i>\n')
            return True
        except:
            return False
    
    def writeDmToXml(fileName:str, xmlDmList: list):
        """
        写入列表内容到文件
        """
        filePath = os.path.join(outputDir, fileName)
        with open(filePath, 'a', encoding='UTF-8') as f:
            for dm in xmlDmList:
                f.write(dm)

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