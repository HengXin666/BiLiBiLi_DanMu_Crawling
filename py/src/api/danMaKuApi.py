# -*- coding: utf-8 -*-
import random
import requests
from enum import Enum
from dataclasses import dataclass
from typing import Optional, List

from .pb import dm_pb2 as Danmaku
from .pb import basDm_pb2 as BasDanmaku

class DmColorfulType(Enum):
    NoneType = 0            # 无
    VipGradualColor = 60001 # 渐变色

@dataclass
class DanmakuElem:
    id: int                      # 弹幕 dmid (唯一)
    progress: int                # 弹幕出现位置(ms)
    mode: int                    # 弹幕类型
    fontsize: int                # 弹幕字号
    color: int                   # 弹幕颜色
    midHash: str                 # 发送者 mid hash
    content: str                 # 弹幕正文
    ctime: int                   # 发送时间
    action: str                  # 动作
    pool: int                    # 弹幕池
    idStr: str                   # 弹幕 dmid str
    attr: int                    # 弹幕属性位 (主要是看是否为 `保护弹幕` (& 1 得到))
    weight: int                  # 权重 [1,10] 此次假设 0 就是无效的 (py protobuf 是使用默认值填充, 无法判断是否解析不到)
                                 # https://github.com/SocialSisterYi/bilibili-API-collect/issues/1331
    animation: Optional[str] = None              # 动画, 字段号22, protobuf可选
    colorful: Optional["DmColorfulType"] = None  # 大会员专属颜色, 字段号24, 可选

def convertElem(pbElem: Danmaku.DanmakuElem) -> DanmakuElem: # type: ignore
    return DanmakuElem(
        id=pbElem.id,
        progress=pbElem.progress,
        mode=pbElem.mode,
        fontsize=pbElem.fontsize,
        color=pbElem.color,
        midHash=pbElem.midHash,
        content=pbElem.content,
        ctime=pbElem.ctime,
        weight=pbElem.weight,
        action=pbElem.action,
        pool=pbElem.pool,
        idStr=pbElem.idStr,
        attr=pbElem.attr,
        animation=pbElem.animation if pbElem.animation != "" else None,
        colorful=DmColorfulType(pbElem.colorful) if pbElem.colorful != 0 else None
    )

def convertReply(pbReply: Danmaku.DmSegMobileReply) -> List[DanmakuElem]: # type: ignore
    return [convertElem(e) for e in pbReply.elems]

class DanMaKuApi:
    _headers = {
        'User-Agent': 'Modified-Since,Pragma,Last-Modified,Cache-Control,Expires,Content-Type,Access-Control-Allow-Credentials,DNT,X-CustomHeader,Keep-Alive,User-Agent,X-Cache-Webcdn,X-Bilibili-Key-Real-Ip,X-Upos-Auth,Range'
    }

    def __init__(self, cookies: List[str], timeout: int = 10) -> None:
        """初始化

        Args:
            cookies (List[str]): 凭证列表
            timeout (int, optional): 超时时间. Defaults to 10.
        """
        self._cookies = cookies
        self._timeout = timeout

    def _getAnyOneCookies(self) -> dict:
        """
        从凭证列表中随机获取一个Cookies
        """
        return {
            'SESSDATA': random.choice(self._cookies)
        }

    def deserializeDanMaKu(self, data) -> list[DanmakuElem]:
        """
        反序列化 普通分段包弹幕
        """
        danmakuSeg = Danmaku.DmSegMobileReply() # type: ignore
        danmakuSeg.ParseFromString(data)
        return convertReply(danmakuSeg)

    def getHistoricalDanMaKu(self, date: str, cid: int) -> list[DanmakuElem]:
        """
        获取历史弹幕
            - date: 需要获取的日期, 如`2017-01-21`
        
        返回值
            - list[DanmakuElem], 每一项是弹幕数据
        """
        url = 'https://api.bilibili.com/x/v2/dm/web/history/seg.so'
        params = {
            'type': 1,   # 弹幕类型 (1: 视频弹幕)
            'oid': cid,  # cid  1176840
            'date': date # 弹幕日期
        }
        resp = requests.get(
            url,
            params,
            cookies=self._getAnyOneCookies(),
            headers=self._headers,
            timeout=self._timeout
        )
        return self.deserializeDanMaKu(resp.content)

    def getBasDanMaKu(self, cid: int) -> list[tuple]:
        """
        获取BAS弹幕专包
        """
        url = f'https://api.bilibili.com/x/v2/dm/web/view?type=1&oid={cid}'
        data = requests.get(
            url,
            cookies=self._getAnyOneCookies(),
            headers=self._headers,
            timeout=self._timeout
        )
        target = BasDanmaku.DmWebViewReply() # type: ignore
        target.ParseFromString(data.content)
        res = []
        for i_url in target.specialDms:
            # 使用普通分段包弹幕的proto结构体反序列化此bin数据
            res.extend(
                self.deserializeDanMaKu(
                    requests.get(
                        i_url, 
                        cookies=self._getAnyOneCookies(),
                        headers=self._headers,
                        timeout=self._timeout
                    ).content
                )
            )
        return res

    def getRealTimeDanMaKu(self, segmentIndex: int, cid: int) -> list[DanmakuElem]:
        """
        获取实时弹幕
            - segmentIndex: 仅获取 6min 的整数倍时间内的弹幕, 6min 内最多弹幕数为 6000 条
                            (如第一包中弹幕progress值域为0-360000)
        
        返回值
            - list[DanmakuElem], 每一项是弹幕数据
        """
        url = 'https://api.bilibili.com/x/v2/dm/web/seg.so'
        params = {
            'type': 1,                    # 弹幕类型 (1: 视频弹幕)
            'oid': cid,                   # cid  1176840
            'segment_index': segmentIndex # 弹幕分段
        }
        resp = requests.get(
            url, 
            params,
            cookies=self._getAnyOneCookies(),
            headers=self._headers,
            timeout=self._timeout
        )
        return self.deserializeDanMaKu(resp.content)