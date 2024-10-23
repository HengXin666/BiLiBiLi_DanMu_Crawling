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
    'SESSDATA': 'b5a3b2e7%2C1737681226%2Ccda43%2A71CjABTI92O0FK36mvsWSgR8qb-at00NnH-H3sVEA71wfSeCBw2N7gpaV1xWkuoyoFlewSVnluczBVMXJVX0RRdFRPbWRXR0VJeS1mV0xMbVJlOW5pXzRFVTZET1hPMmVqWEpsYVZnbTE2eklmVEM5WXpXOTQxeDZhTVpTSWxpUy1obUVEZVNmM1l3IIEC'
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
        'Cookie': 'SESSDATA=' + 'b5a3b2e7%2C1737681226%2Ccda43%2A71CjABTI92O0FK36mvsWSgR8qb-at00NnH-H3sVEA71wfSeCBw2N7gpaV1xWkuoyoFlewSVnluczBVMXJVX0RRdFRPbWRXR0VJeS1mV0xMbVJlOW5pXzRFVTZET1hPMmVqWEpsYVZnbTE2eklmVEM5WXpXOTQxeDZhTVpTSWxpUy1obUVEZVNmM1l3IIEC' + ';',
        'Referer': 'https://www.bilibili.com/',
        'Origin': 'https://www.bilibili.com'
    }

    resp = requests.get(url=url, params=params, headers=req_headers)

    data = resp.content
    danmaku_seg = Danmaku.DmSegMobileReply()
    danmaku_seg.ParseFromString(data)

    for i in range(len(danmaku_seg.elems)):
        print(text_format.MessageToString(danmaku_seg.elems[i], as_utf8=True), end='\n\n')

# date: '2017-01-21'
def 获取历史弹幕(date: str):
    url = 'https://api.bilibili.com/x/v2/dm/web/history/seg.so'
    params = {
        'type': 1,           # 弹幕类型 (1: 视频弹幕)
        'oid': 3262388,      # cid  1176840
        'date': date         # 弹幕日期
    }
    resp = requests.get(url, params, cookies=cookies, headers=UserAgent)
    data = resp.content
    print(data)

    danmaku_seg = Danmaku.DmSegMobileReply()
    danmaku_seg.ParseFromString(data)

    for i in range(len(danmaku_seg.elems)):
        print(text_format.MessageToString(danmaku_seg.elems[i], as_utf8=True), end='\n\n')
    
    print("获取日期: ", params['date'])
    print("获取条数: ", len(danmaku_seg.elems))

def 获取BAS弹幕专包():
    AVID = 2
    CID = 62131
    url = f'https://api.bilibili.com/x/v2/dm/web/view?type=1&oid={CID}&pid={AVID}'

    data = requests.get(url, cookies=cookies, headers=UserAgent)
    target = BasDanmaku.DmWebViewReply()
    target.ParseFromString(data.content)
    print(data.content)
    print(f'特殊弹幕包数={len(target.specialDms)}')
    for i in target.specialDms:
        print(f'特殊弹幕包url={i}')
    # 使用普通分段包弹幕的proto结构体反序列化此bin数据

if __name__ == '__main__':
    pass