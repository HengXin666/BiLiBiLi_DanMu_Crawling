# -*- coding: utf-8 -*-
import threading
import time
import random
from ..utils import configUtils
from ..utils import yearDaysUtils

def singleton(cls):
    instances = {}
    lock = threading.Lock()

    def get_instance(*args, **kwargs):
        if cls not in instances:
            with lock:
                if cls not in instances:
                    instances[cls] = cls(*args, **kwargs)
        return instances[cls]
    return get_instance

@singleton
class ReqDataSingleton:
    """
    请求信息单例
    """
    _instance = None

    def __init__(self):
        """
        构造函数
        """
        self.UserAgent = { # 现代反爬, 不屑于从'User-Agent'判断, 所以有就可以了..
            'User-Agent': 'Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range'
        }
        self.cookies = [] # 初始化凭证列表
        self.cid = -1
        self.startDate = '2009-06-26' # 爬取的起始日期
        self.endDate = time.strftime("%Y-%m-%d", time.localtime()) # 终止日期
        self.isGetAllDanMaKu = True
        self.isGetToNowTime = True
        self.isGetOptimize = True # 是否开启爬取优化

        ### 爬取间隔时间 [timerMin, timerMax] ###
        self.timerMin = 8
        self.timerMax = 15
        ### 不持久化, 只能每一次配置, 防止遗忘! ###

        self.init(configUtils.loadConfig())
    
    def init(self, data: dict):
        self.UserAgent = data.get('net', {}).get('UserAgent', {})
        self.cookies = data.get('net', {}).get('cookies', [])  # 初始化凭证列表
        self.cid = data.get('settings', {}).get('cid', -1)
        self.startDate = data.get('settings', {}).get('startDate', '2009-06-26')  # 爬取的起始日期
        self.endDate = data.get('settings', {}).get('endDate', time.strftime("%Y-%m-%d", time.localtime()))  # 终止日期
        self.isGetAllDanMaKu = bool(data.get('settings', {}).get('isGetAllDanMaKu', True))
        self.isGetToNowTime = bool(data.get('settings', {}).get('isGetToNowTime', True))
        self.isGetOptimize = bool(data.get('settings', {}).get('isGetOptimize', True))
        
        self.yearList = None # yearDaysUitls.YearFamily(2009, int(time.strftime("%Y", time.localtime())))
        if (len(data.get('run', {}).get('yearFamily', {}).get("list", [])) > 0):
            self.yearList = yearDaysUtils.YearFamily.fromJson(data.get('run', {}).get('yearFamily', {}))
        self.outFile = data.get('run', {}).get('outFile', "danmaku.xml")

        if self.isGetToNowTime:
            self.endDate = time.strftime("%Y-%m-%d", time.localtime())

    def get_members(self):
        # 返回类的所有成员作为字典
        return {key: value for key, value in vars(self).items()}

    def save(self):
        """保存"""
        defaultConfig = {
            'settings': {
                'cid': self.cid,
                'startDate': self.startDate,
                'endDate': self.endDate,
                'isGetAllDanMaKu': self.isGetAllDanMaKu,  # 获取全弹幕
                'isGetToNowTime': self.isGetToNowTime,    # 获取直到当前时间 
                'isGetOptimize': self.isGetToNowTime,     # 爬取优化
            },
            'net': {
                'UserAgent': self.UserAgent,
                'cookies': self.cookies,
            },
            'run': {
                'outFile': self.outFile, # 保存的文件名称
                'yearFamily': self.yearList.toJsonDict() if self.yearList != None else {
                    "list": [],
                    "nowAllIndex": -1
                }, # 爬取记录
            }
        }
        configUtils.writeConfig(defaultConfig)

    def getAnyOneCookies(self) -> dict:
        """
        从凭证列表中随机获取一个Cookies
        """
        return {
            'SESSDATA': random.choice(self.cookies)
        }

if __name__ == '__main__':
    # 使用示例
    singleton1 = ReqDataSingleton()
    singleton2 = ReqDataSingleton()
    print(singleton1 is singleton2)  # 输出 True，表示是同一个实例
    print(ReqDataSingleton().cid)
    ReqDataSingleton().cid = 114
    print(ReqDataSingleton().cid)
