from typing import List
from fastapi import APIRouter
from pydantic import BaseModel

from ..api.videoApi import VideoApi, VideoPart
from ..pojo.vo.ResponseModel import ResponseModel

videoInfoController = APIRouter(prefix="/videoInfo")

class VideoPartListVo(BaseModel):
    cidList: List[VideoPart]

class UrlVo(BaseModel):
    url: str

@videoInfoController.post("/getVideoPartList", response_class=ResponseModel[VideoPartListVo])
def getVideoPartList(urlVo: UrlVo):
    """查询cid

    Args:
        urlVo (UrlVo): url 或者 cid=\\d+

    Returns:
        _type_: 视频元数据
    """
    err, resList = VideoApi.getCidPart(urlVo.url)
    if (err == 0):
        return ResponseModel.success({
            "cidList": resList
        })
    else:
        return ResponseModel.error(code=err, msg={
            -1:   "解析失败",
            -400: "请求错误",
            -404: "无视频"
        }[err])