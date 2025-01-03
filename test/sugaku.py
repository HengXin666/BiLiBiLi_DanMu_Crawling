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
        day += jp
    
    return res, len(pool.vis)

def generate_danmaku_data(pool_size: int, date_len: int, n: int, move_weight: float = 0.7) -> List[List[int]]:
    """
    生成随机二维弹幕数组，使用滑动窗口思想，确保覆盖整个ID范围。
    :param pool_size: 每日弹幕数量
    :param date_len: 天数
    :param n: 弹幕ID范围 [1, n]
    :param move_weight: 滑动窗口移动的概率权重
    :return: 二维弹幕数组
    """
    ids = list(range(1, n + 1))
    res = []
    window_start = 0
    window_size = pool_size
    step = max(n // date_len, 1)  # 确保滑动完

    for day in range(date_len):
        # 确保窗口滑动
        if random.random() < move_weight and window_start + window_size < n:
            window_start += min(step, n - window_start - window_size)

        window = ids[window_start : window_start + window_size]
        res.append(random.sample(window, len(window)))

        # 保证覆盖所有ID范围
        if window_start + window_size >= n:
            window_start = max(0, window_start - step)

    return res

# 测试程序
def main():
    # 生成随机弹幕数据
    pool_size = 3000
    date_len = 300
    n = 10 ** 5 # 10w

    for i in range(1, 10):
        print(f"\n当平均新增弹幕率为 {format(0.1 * i, '.2f')} 时候的情况:")
        random_danmaku = generate_danmaku_data(pool_size, date_len, n, 0.1 * i)
        # print("生成的随机弹幕数据:", random_danmaku)
        honmonoDanmaku = reference(random_danmaku)
        print(f"共有弹幕: {len(random_danmaku) * len(random_danmaku[0])} 条, 真弹幕数为 {honmonoDanmaku} 条")

        # 调用爬取模拟函数
        total_calls, addCnt = simulate_danmaku_crawl(DanmakuPool(random_danmaku))
        print(f"爬取函数调用次数: {total_calls} | 爬取率: {format(addCnt / honmonoDanmaku * 100, '.2f')} % {"" if honmonoDanmaku == addCnt else f"剩余 {honmonoDanmaku - addCnt} 条弹幕未爬取"} | 性能提升: {format(date_len / total_calls * 100 - 100, '.2f')} %")

if __name__ == "__main__":
    main()
