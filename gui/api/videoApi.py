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
    
"""
md号
打开番剧的详情页，对应网址中包含md号：https://www.bilibili.com/bangumi/media/md28229233

ss号
打开番剧索引或我的追番中的番剧，对应的网址中包含ss号：https://www.bilibili.com/bangumi/play/ss33802

ep号
打开番剧的某一集，对应网址中包含ep号：https://www.bilibili.com/bangumi/play/ep330798
"""
def 获取番剧信息(mdId = None, ssId = None, epId = None) -> tuple[int, list]:
    """
    获取番剧信息

    mdId, ssId, epId 选择性传递一个即可

    return:  (状态码, 视频分P信息列表)

    其中, 状态码值只能为:
        0: 成功
        -400: 请求错误
        -404: 无视频

    视频分P信息列表 = dict:
        cid: 番剧cid,
        page: 分p,
        part: 标题
    """

    if (mdId != None):
        url = "https://api.bilibili.com/pgc/review/user"
        params = {
            'media_id': mdId
        }
        resp = requests.get(url, params, headers=ReqDataSingleton().UserAgent)
        ssId = resp.json()['result']['media']['season_id']

    url = "https://api.bilibili.com/pgc/view/web/season"
    params = {
        'ep_id': epId 
    } if epId != None else {
        'season_id': ssId
    }
    resp = requests.get(url, params, headers=ReqDataSingleton().UserAgent)
    data = resp.json()
    res = []
    try:
        for it in data['result']['episodes']:
            i_dict = {}
            i_dict['cid'] = it['cid']   # 视频cid
            i_dict['page'] = 0          # 分p
            i_dict['part'] = it['share_copy'] # 标题
            res.append(i_dict)
    finally:
        return data['code'], res