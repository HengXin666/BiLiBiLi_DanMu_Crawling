# -*- coding: utf-8 -*-
import requests
import google.protobuf.text_format as text_format
from .pb import dm_pb2 as Danmaku
from .pb import basDm_pb2 as BasDanmaku
from .reqDataSingleton import ReqDataSingleton

# 设置代理
# proxies = {
#     'http': 'socks5h://127.0.0.1:2333',
#     'https': 'socks5h://127.0.0.1:2333'
# }

"""
# 废弃的, 留着吧... (获取实时弹幕)
def 获取弹幕():
         #'https://api.bilibili.com/x/v2/dm/web/history/seg.so'
    url = 'https://api.bilibili.com/x/v2/dm/web/seg.so'
    params = {
        'type': 1,         # 弹幕类型
        'oid': 1176840,    # cid
        'pid': 810872,     # avid
        'segment_index': 1 # 弹幕分段
    }

    req_headers = {
        'User-Agent': 'Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range',
        'Cookie': 'SESSDATA=' + '???' + ';',
        'Referer': 'https://www.bilibili.com/',
        'Origin': 'https://www.bilibili.com'
    }

    resp = requests.get(url=url, params=params, headers=req_headers)

    data = resp.content
    danmaku_seg = Danmaku.DmSegMobileReply()
    danmaku_seg.ParseFromString(data)

    for i in range(len(danmaku_seg.elems)):
        print(text_format.MessageToString(danmaku_seg.elems[i], as_utf8=True), end='\n\n')
"""

def DeserializeNormalSegmentedPacketDanMaKu(data) -> list[tuple]:
    """
    反序列化 普通分段包弹幕
    """
    danmakuSeg = Danmaku.DmSegMobileReply()
    danmakuSeg.ParseFromString(data)
    def _extractInfo(it):
        return (
            it.progress / 1000.0,   #0 progress <-> 出现时间 # (需要 / 1000) xml为float类型
            it.mode,                #1 mode <-> 弹幕类型
            it.fontsize,            #2 fontsize <-> 弹幕字号
            it.color,               #3 color <-> 弹幕颜色
            it.ctime,               #4 ctime <-> 弹幕发送时间
            it.pool,                #5 pool <-> 弹幕池类型 (0 普通, 2 bas)
            it.midHash,             #6 midHash <-> 发送者mid的HASH
            it.id,                  #7 id <-> 弹幕dmid: int32 唯一的!
            it.content              #8 content <-> 弹幕内容
        )
    return list(map(_extractInfo, danmakuSeg.elems))

def getHistoricalDanMaKu(date: str, cid: int) -> list[tuple]:
    """
    获取历史弹幕
        - date: 需要获取的日期, 如`2017-01-21`
    
    返回值
        - list[元组], 每一项是弹幕数据
    """
    url = 'https://api.bilibili.com/x/v2/dm/web/history/seg.so'
    params = {
        'type': 1,   # 弹幕类型 (1: 视频弹幕)
        'oid': cid,  # cid  1176840
        'date': date # 弹幕日期
    }
    resp = requests.get(url, params, cookies=ReqDataSingleton().getAnyOneCookies(), headers=ReqDataSingleton().UserAgent, timeout=10)
    data = resp.content

    """
    <d p="490.19100,1,25,16777215,1584268892,0,a16fe0dd,29950852386521095">从结尾回来看这里，更感动了！</d>
    """
    return DeserializeNormalSegmentedPacketDanMaKu(data)

def getBasDanMaKu(cid: int) -> list[tuple]:
    """
    获取BAS弹幕转包
    """
    url = f'https://api.bilibili.com/x/v2/dm/web/view?type=1&oid={cid}'
    data = requests.get(url, cookies=ReqDataSingleton().getAnyOneCookies(), headers=ReqDataSingleton().UserAgent, timeout=10)
    target = BasDanmaku.DmWebViewReply()
    target.ParseFromString(data.content)
    res = []
    print(f'特殊弹幕包数={len(target.specialDms)}')
    for i_url in target.specialDms:
        # 使用普通分段包弹幕的proto结构体反序列化此bin数据
        print(i_url)
        res.extend(
            DeserializeNormalSegmentedPacketDanMaKu(
                requests.get(
                    i_url, 
                    cookies=ReqDataSingleton().getAnyOneCookies(),
                    headers=ReqDataSingleton().UserAgent,
                    timeout=10
                ).content
            )
        )
    return res

# 测试
# def dmTest():
#     with open("C:\\Users\\Heng_Xin\\Downloads\\dm.bin", "rb") as f:
#         data = f.read()
#     return DeserializeNormalSegmentedPacketDanMaKu(data)

"""
废弃!

# dateMonth 格式: '2024-09'
def 查询某月份有历史弹幕的天数列表(dateMonth: str):
    url = 'https://api.bilibili.com/x/v2/dm/history/index'
    params = {
        'type': 1,
        'oid': 1176840,
        'month': dateMonth
    }
    # resp = requests.get(url, params, cookies=cookies, proxies=proxies, headers=UserAgent)
# 查询某月份有历史弹幕的天数列表('2021-07')
"""

if __name__ == '__main__':
    getBasDanMaKu(1176840)