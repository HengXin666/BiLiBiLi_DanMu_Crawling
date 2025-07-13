import sys
import os

# 获取 src 目录的绝对路径
__srcPath__ = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, __srcPath__)