from fastapi import APIRouter

from ..pojo.vo.ResponseModel import ResponseModel

basicInfoController = APIRouter(prefix="/base")


@basicInfoController.get("/ping", response_model=ResponseModel[str])
def ping():
    """检测服务可用性

    Returns:
        _type_: ok
    """
    return ResponseModel.success("pong")
