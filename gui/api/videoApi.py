# -*- coding: utf-8 -*-
import requests
from .reqDataSingleton import ReqDataSingleton

def 获取视频信息(bvid: str) -> tuple[int, list]:
    """
    获取视频信息

    `return`: (状态码, 视频分P信息列表)

    其中, 状态码值只能为:
        0: 成功
        -400: 请求错误
        -404: 无视频

    视频分P信息列表 = dict:
        cid: 视频cid,
        page: 分p,
        part: 标题
    """
    url = 'https://api.bilibili.com/x/player/pagelist'
    params = {
        'bvid': bvid
    }
    resp = requests.get(url, params, headers=ReqDataSingleton().UserAgent)
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