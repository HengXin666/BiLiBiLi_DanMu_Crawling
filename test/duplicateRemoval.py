import re

def openDanMakuFileGetSet(filePath: str):
    def read_xml():
        with open(filePath, "r", encoding="UTF-8") as f:
            return f.readlines() # 获取文件每一行内容,保存为列表
    reAll = re.compile(r'(<d p="\d+\.\d+,\d,\d+,\d+,\d+,\d,.+?,\d+">.+?</d>)')
    reDmMid = re.compile('<d p="\\d+\\.\\d+,\\d,\\d+,\\d+,\\d+,\\d,[A-Za-z0-9]+,(\\d+)">.*?</d>') # 弹幕id
    res = set()
    for line in read_xml():
        dmList = re.findall(reAll, line)
        for dm in dmList:
            # 读取每一条弹幕
            dmMid = re.findall(reDmMid, dm)[0]
            dmMid = int(dmMid)
            if dmMid not in res:
                # 去重结果
                res.add(dmMid)
    return res

def main():
    experimentalSetFilePath = ".\\output\\性能爬取炮姐2.0.xml" # 实验集文件路径
    verifySetFilePath = ".\\output\\danmaku.xml" # 验证集文件路径

    s1 = openDanMakuFileGetSet(experimentalSetFilePath)
    print(f"实验集len = {len(s1)}")
    s2 = openDanMakuFileGetSet(verifySetFilePath)
    print(f"验证集len = {len(s2)}")

    res = s2 - s1
    print(len(res))
    print(res)

if __name__ == "__main__":
    main()