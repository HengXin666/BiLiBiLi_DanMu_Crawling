from fastapi import APIRouter
from src.info.version import get_version

from ..pojo.vo.ResponseModel import ResponseModel

basicInfoController = APIRouter(prefix="/base")


@basicInfoController.get("/ping", response_model=ResponseModel[str])
def ping():
    """检测服务可用性

    Returns:
        _type_: ok
    """
    return ResponseModel.success("pong")


@basicInfoController.get("/version", response_model=ResponseModel[str | None])
def version():
    """获取版本信息

    Returns:
        _type_: 版本信息
    """
    ver = get_version()
    return ResponseModel.success(ver)
