# -*- coding: utf-8 -*-
import threading

# 单例注解
def Singleton(cls):
    instances = {}
    lock = threading.Lock()

    def getInstance(*args, **kwargs):
        if cls not in instances:
            with lock:
                if cls not in instances:
                    instances[cls] = cls(*args, **kwargs)
        return instances[cls]
    
    return getInstance