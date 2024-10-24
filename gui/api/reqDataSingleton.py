# -*- coding: utf-8 -*-
import threading
import time

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

    """
    构造函数
    """
    def __init__(self):
        self.UserAgent = { # 现代反爬, 不屑于从'User-Agent'判断, 所以有就可以了..
            'User-Agent': 'Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range'
        }
        self.cookies = [] # 初始化凭证列表
        self.cid = -1
        self.startDate = '2009-06-26' # 爬取的起始日期
        self.endDate = time.strftime("%Y-%m-%d", time.localtime()) # 终止日期

    """
    从凭证列表中随机获取一个Cookies
    """
    def getAnyOneCookies(self) -> dict:
        return {
            'SESSDATA': round(self.cookies)
        }

if __name__ == '__main__':
    # 使用示例
    singleton1 = ReqDataSingleton()
    singleton2 = ReqDataSingleton()
    print(singleton1 is singleton2)  # 输出 True，表示是同一个实例
    ReqDataSingleton().cid = 114
    print(ReqDataSingleton().cid)
