import requests


# dateMonth 格式: '2024-09'
def 查询某月份有历史弹幕的天数列表(dateMonth: str):
    # https://api.bilibili.com/x/v2/dm/history/index?type=1&oid=1176840&month=2021-12
    url = 'https://api.bilibili.com/x/v2/dm/history/index'
    params = {
        'type': 1,
        'oid': 1176840,
        'month': dateMonth
    }
    resp = requests.get(url, params, cookies=cookies, proxies=proxies, headers=UserAgent)
    print(resp.content)
# 查询某月份有历史弹幕的天数列表('2021-07')

"""
返回: (状态码, 视频分P信息列表)

其中, 状态码值只能为:
    0: 成功
    -400: 请求错误
    -404: 无视频
"""
def 获取视频信息(bvid: str) -> tuple[int, list]:
    url = 'https://api.bilibili.com/x/player/pagelist'
    params = {
        'bvid': bvid
    }
    resp = requests.get(url, params, headers=UserAgent)
    data = resp.json()
    res = []
    try:
        for it in data['data']:
            i_dict = {}
            i_dict['cid'] = it['cid']   # 视频cid
            i_dict['page'] = it['page'] # 分p
            i_dict['part'] = it['part'] # 标题
            res.append(i_dict)
    finally:
        return data['code'], res

"""
简单架构:
    - 使用二分确认日期 {实际上获取发售日期即可?}
        - 例如: av314, 它的发布日期是`2012-08-19`, 但是`2012-07-18`都有弹幕!
        - 保险起见, 二分为妙!
    - 使用增量日志以替代原滚动日志
    - 本日志采用为{日期: bool}映射模式 (代码弹幕呢?!)
    - 使用UI界面
    - 保存上一次的设置

    - UID 确认
    - 视频cid 转换

    - 超低的cpu占用, 内存次之

可能增加:
    - 扫码登录
"""

if __name__ == '__main__':
    # 获取历史弹幕('2012-07-18')
    print(获取视频信息('BV1Js411o76u'))

"""
https://api.bilibili.com/x/player/pagelist?aid=314

{
    "code": 0,
    "message": "0",
    "ttl": 1,
    "data": [
        {
            "cid": 3262388,
            "page": 1,
            "from": "vupload",
            "part": "P1",
            "duration": 96,
            "vid": "",
            "weblink": "",
            "dimension": {
                "width": 480,
                "height": 360,
                "rotate": 0
            }
        }
    ]
}

look: https://github.com/SocialSisterYi/bilibili-API-collect/blob/master/docs/video/info.md
"""