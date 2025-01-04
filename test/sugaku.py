import math
from typing import List, Set
import random

# 弹幕处理模块
class DanmakuPool:
    def __init__(self, dm: List[List[int]]):
        self._dm = dm  # 弹幕数据 (测试使用的)
        self.vis = set() # 已经爬取的弹幕

    def getDanmaku(self, date: int) -> int:
        """
        模拟爬取第date天的弹幕, 并且去重, 得到新增的弹幕数量
        新增的弹幕数量
        """
        res = 0
        for it in self._dm[date]:
            if it not in self.vis:
                self.vis.add(it)
                res += 1
        return res
    
    def getPoolSize(self) -> int:
        """
        获取弹幕池容量
        """
        return len(self._dm)
        
def reference(dm: List[List[int]]) -> int:
    """返回去重后的弹幕数量

    Args:
        dm (List[List[int]]): _description_

    Returns:
        int: _description_
    """
    res = set()
    for it in dm:
        res |= set(it)
    return len(res)

# 数学模型爬取模块
def simulate_danmaku_crawl(pool: DanmakuPool) -> tuple[int, int]:
    """
    使用动态调整跳跃策略模拟爬取弹幕。
    返回爬取函数的调用次数。
    """
    # 初始化爬取相关变量
    res = 0
    poolSize = pool.getPoolSize()
    day = 0
    jp = 1
    maeDay = 0
    while day < poolSize:
        res += 1
        getCnt = pool.getDanmaku(day)
        if getCnt == poolSize:
            # 必需后退
            day = maeDay + ((day - maeDay) >> 1)
            jp >>= 1
        elif getCnt >= poolSize * 0.8:
            jp -= int(max(jp, jp * 0.25))
        else:
            if getCnt != 0:
                jp += int((poolSize / getCnt) * 0.5)
            else:
                jp += 1

        maeDay = day
        day = min(day + jp, poolSize - (day != poolSize - 1))
    
    return res, len(pool.vis)

from typing import List
import math

def generateDanmakuData(
        poolSize: int, 
        dateLen: int, 
        n: int, 
        moveWeight: float = 0.7
    ) -> List[List[int]]:
    """
    生成随机二维弹幕数组, 使用滑动窗口思想, 确保覆盖整个ID范围.
    :param poolSize: 每日弹幕数量
    :param dateLen: 天数
    :param n: 弹幕总数量
    :param moveWeight: 滑动窗口移动的概率权重
    :return: 二维弹幕数组
    """
    # 初始化所有 ID
    idx = list(range(1, n + 1))
    res = []
    
    # 计算新增和延续的数量
    new_count = math.ceil(poolSize * moveWeight)
    old_count = poolSize - new_count
    
    # 当前窗口起点
    window_start = 0
    
    for _ in range(dateLen):
        # 获取新增部分的起点和终点
        new_start = window_start + old_count
        new_end = new_start + new_count
        
        # 确保新增部分不超出范围
        if new_start >= n:
            new_ids = []  # 无法新增更多弹幕
        else:
            new_ids = idx[new_start:new_end]
        
        # 确保延续部分不超出范围
        old_ids = idx[window_start:window_start + old_count]
        
        # 拼接老部分和新部分
        current_window = old_ids + new_ids
        
        # 如果当前窗口不足 poolSize, 从头部补足
        if len(current_window) < poolSize:
            current_window += idx[:poolSize - len(current_window)]
        
        # 截取当前窗口
        res.append(current_window[:poolSize])
        
        # 滑动窗口的起点
        window_start += new_count
        
        # 如果窗口超出范围, 停止滑动
        if window_start >= n:
            window_start = n - poolSize  # 固定在最后的位置
    return res

# 测试程序
def main():
    # 生成随机弹幕数据
    pool_size = 3000
    date_len = 300
    n = 9 * 10 ** 5 # 100w ?? 3000 * 300=90w

    for i in range(1, 10):
        print(f"\n当平均新增弹幕率为 {format(i * 0.1, '.2f')} % 时候的情况:")
        random_danmaku = generateDanmakuData(pool_size, date_len, n, i * 0.1)
        # print("生成的随机弹幕数据:", random_danmaku)
        honmonoDanmaku = reference(random_danmaku)
        print(f"共有弹幕: {len(random_danmaku) * len(random_danmaku[0])} 条, 真弹幕数为 {honmonoDanmaku} 条 | 占比 {format(honmonoDanmaku / (len(random_danmaku) * len(random_danmaku[0])) * 100, '.2f')} %")

        # 调用爬取模拟函数
        total_calls, addCnt = simulate_danmaku_crawl(DanmakuPool(random_danmaku))
        print(f"爬取函数调用次数: {total_calls} | 爬取率: {format(addCnt / honmonoDanmaku * 100, '.2f')} % {"" if honmonoDanmaku == addCnt else f"剩余 {honmonoDanmaku - addCnt} 条弹幕未爬取"} | 性能提升: {format(date_len / total_calls * 100 - 100, '.2f')} %")

if __name__ == "__main__":
    main()
