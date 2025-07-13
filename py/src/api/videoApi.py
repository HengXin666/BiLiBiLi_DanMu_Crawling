# -*- coding: utf-8 -*-
import requests
from dataclasses import dataclass
from typing import Tuple, List

from .danMaKuApi import DanMaKuApi
from ..utils.cidUtils import CidUtils

@dataclass
class VideoPart:
    """视频分P信息"""
    cid: int   # 视频 cid
    page: int  # 分P编号
    part: str  # 标题

class VideoApi:
    @staticmethod
    def _getVideoInfo(bvid: str) -> Tuple[int, List[VideoPart]]:
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
        resp = requests.get(url, params, headers=DanMaKuApi._headers)
        data = resp.json()
        res: List[VideoPart] = []
        try:
            for it in data['data']:
                res.append(VideoPart(
                    cid=it['cid'],
                    page=it['page'],
                    part=it['part']
                ))
        finally:
            return data['code'], res

    """
    md号
    打开番剧的详情页, 对应网址中包含md号: https://www.bilibili.com/bangumi/media/md28229233

    ss号
    打开番剧索引或我的追番中的番剧, 对应的网址中包含ss号: https://www.bilibili.com/bangumi/play/ss33802

    ep号
    打开番剧的某一集, 对应网址中包含ep号: https://www.bilibili.com/bangumi/play/ep330798
    """
    @staticmethod
    def _getANiMeInfo(mdId= None, ssId = None, epId = None) -> Tuple[int, List[VideoPart]]:
        """
        获取番剧信息

        mdId, ssId, epId 选择性传递一个即可

        return: (状态码, 视频分P信息列表)

        其中, 状态码值只能为:
            0: 成功
            -400: 请求错误
            -404: 无视频

        视频分P信息列表 = dict:
            cid: 番剧cid,
            page: 分p,
            part: 标题
        """

        if mdId is not None:
            url = "https://api.bilibili.com/pgc/review/user"
            params = {
                'media_id': mdId
            }
            resp = requests.get(url, params, headers=DanMaKuApi._headers)
            ssId = resp.json()['result']['media']['season_id']

        url = "https://api.bilibili.com/pgc/view/web/season"
        params = { 'ep_id': epId } if epId is not None else { 'season_id': ssId }
        resp = requests.get(url, params, headers=DanMaKuApi._headers)
        data = resp.json()
        res: List[VideoPart] = []
        try:
            for it in data['result']['episodes']:
                res.append(VideoPart(
                    cid=it['cid'],
                    page=0,
                    part=it['share_copy']
                ))
        finally:
            return data['code'], res

    @staticmethod
    def getCidPart(url: str) -> Tuple[int, List[VideoPart]]:
        """获取视频信息数组

        Args:
            url (str): 视频链接

        Returns:
            Tuple[int, List[VideoPart]]: (状态码, 视频分P信息列表)

        其中, 状态码值只能为:
            0: 成功
            -1:   解析失败
            -400: 请求错误
            -404: 无视频

        视频分P信息列表 = dict:
            cid: 番剧cid,
            page: 分p,
            part: 标题
        """
        try:
            bv = CidUtils.extractBv(url)
            anime = CidUtils.extractANiMe(url)
            if (bv != None):
                # 解析普通视频
                return VideoApi._getVideoInfo(bv)
            elif (anime['mdId'] != None 
               or anime['ssId'] != None
               or anime['epId'] != None):
                # 解析番剧
                return VideoApi._getANiMeInfo(
                    mdId=anime['mdId'],
                    ssId=anime['ssId'],
                    epId=anime['epId']
                )
        except:
            pass
        return (-1, [])