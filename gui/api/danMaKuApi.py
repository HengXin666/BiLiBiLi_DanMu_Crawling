import requests
import google.protobuf.text_format as text_format
import pb.dm_pb2 as Danmaku
import pb.basDm_pb2 as BasDanmaku

# 设置代理
proxies = {
    'http': 'socks5h://127.0.0.1:2333',
    'https': 'socks5h://127.0.0.1:2333'
}

cookies = {
    'SESSDATA': '%2C1737681226%2Ccda43%2A71CjABTI92O0FK36mvsWSgR8qb-at00NnH-H3sVEA71wfSeCBw2N7gpaV1xWkuoyoFlewSVnluczBVMXJVX0RRdFRPbWRXR0VJeS1mV0xMbVJlOW5pXzRFVTZET1hPMmVqWEpsYVZnbTE2eklmVEM5WXpXOTQxeDZhTVpTSWxpUy1obUVEZVNmM1l3IIEC'
}

UserAgent = {
    'User-Agent': 'Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range'
}

def text():
    # 设置 URL
    url = "http://i0.hdslb.com/bfs/dm/20cef686d34cedc2491e6452f2b6b9b28189dba6.bin"

    # 获取数据
    resp = requests.get(url)
    data = resp.content

    # 解析数据
    danmaku_seg = Danmaku.DmSegMobileReply()
    danmaku_seg.ParseFromString(data)

    # 打印弹幕
    for i in range(len(danmaku_seg.elems)):
        print(text_format.MessageToString(danmaku_seg.elems[i], as_utf8=True), end='\n\n')

def 获取弹幕():
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

def 反序列化_普通分段包弹幕(data) -> tuple:
    danmakuSeg = Danmaku.DmSegMobileReply()
    danmakuSeg.ParseFromString(data)
    res = []
    for it in danmakuSeg.elems:
        res.append((
                it.progress / 1000.0,   # progress <-> 出现时间 # (需要 / 1000) xml为float类型
                it.mode,                # mode <-> 弹幕类型
                it.fontsize,            # fontsize <-> 弹幕字号
                it.color,               # color <-> 弹幕颜色
                it.ctime,               # ctime <-> 弹幕发送时间
                it.pool,                # pool <-> 弹幕池类型 (0 普通, 2 bas)
                it.midHash,             # midHash <-> 发送者mid的HASH
                it.id,                  # id <-> 弹幕dmid: int32
                it.content              # content <-> 弹幕内容
            )
        )
    return res


# date: '2017-01-21'
def 获取历史弹幕(date: str, cid: int) -> tuple:
    url = 'https://api.bilibili.com/x/v2/dm/web/history/seg.so'
    params = {
        'type': 1,   # 弹幕类型 (1: 视频弹幕)
        'oid': cid,  # cid  1176840
        'date': date # 弹幕日期
    }
    resp = requests.get(url, params, cookies=cookies, headers=UserAgent)
    data = resp.content

    danmakuSeg = Danmaku.DmSegMobileReply()
    danmakuSeg.ParseFromString(data)

    res = []
    for it in danmakuSeg.elems:
        res.append((
                it.progress / 1000.0,   # progress <-> 出现时间 # (需要 / 1000) xml为float类型
                it.mode,                # mode <-> 弹幕类型
                it.fontsize,            # fontsize <-> 弹幕字号
                it.color,               # color <-> 弹幕颜色
                it.ctime,               # ctime <-> 弹幕发送时间
                it.pool,                # pool <-> 弹幕池类型 (0 普通, 2 bas)
                it.midHash,             # midHash <-> 发送者mid的HASH
                it.id,                  # id <-> 弹幕dmid: int32
                it.content              # content <-> 弹幕内容
            )
        )
    """
    <d p="490.19100,1,25,16777215,1584268892,0,a16fe0dd,29950852386521095">从结尾回来看这里，更感动了！</d>
    """
    
    return res

def 获取BAS弹幕专包(cid: int):
    url = f'https://api.bilibili.com/x/v2/dm/web/view?type=1&oid={cid}'

    data = requests.get(url, cookies=cookies, headers=UserAgent)
    target = BasDanmaku.DmWebViewReply()
    target.ParseFromString(data.content)
    print(data.content)
    print(f'特殊弹幕包数={len(target.specialDms)}')
    for i in target.specialDms:
        print(f'特殊弹幕包url={i}')
    # 使用普通分段包弹幕的proto结构体反序列化此bin数据

if __name__ == '__main__':
    获取BAS弹幕专包(1, 1)