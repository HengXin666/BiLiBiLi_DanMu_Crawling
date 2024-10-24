import os
import json
import time

def loadConfig() -> dict:
    """
    加载配置，如果文件不存在则创建默认配置
    """
    config_path = os.path.join(os.path.dirname(__file__), '..', '..', 'config', 'config.json')

    # 检查配置文件是否存在
    if not os.path.exists(config_path):
        # 如果文件不存在，创建默认配置
        defaultConfig = {
            'settings': {
                'cid': -1,
                'startDate': '2009-06-26',
                'endDate': time.strftime("%Y-%m-%d", time.localtime()),
                'isGetAllDanmMaKu': True, # 获取全弹幕
                'isGetToNowTime': True,   # 获取直到当前时间 
            },
            'net': {
                'UserAgent': {
                    "User-Agent": "Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range"
                },
                'cookies': [],
            },
            'run': {
                'outFile': 'danmaku.xml', # 保存的文件名称
                'yearFamily': {
                    'list': [],
                    'nowAllIndex': -1 # 开始爬取日期, -1 代表需要从二分开始
                }, # 爬取记录
            }
        }
        writeConfig(defaultConfig)

    with open(config_path, 'r') as configfile:
        return json.load(configfile)

    return defaultConfig

def writeConfig(config: dict) -> None:
    """
    写入配置
    """
    config_path = os.path.join(os.path.dirname(__file__), '..', '..', 'config', 'config.json')

    with open(config_path, 'w') as configfile:
        json.dump(config, configfile, indent=4)

# 示例使用
if __name__ == "__main__":
    config = loadConfig()

    print(config)

    # 写回配置文件
    writeConfig(config)
