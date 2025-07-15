from fastapi import APIRouter
from pydantic import BaseModel

from ..pojo.vo.ResponseModel import ResponseModel
from ..fileUtils.jsonConfig import GlobalConfig, MainConfig

mainConfigController = APIRouter(prefix="/mainConfig")

class GlobalConfigVo(BaseModel, MainConfig):
    pass

@mainConfigController.get("/getConfig", response_model=ResponseModel[dict])
def getConfig():
    return ResponseModel.success(
        GlobalConfig().get()
    )

@mainConfigController.post("/setConfig", response_class=ResponseModel[None])
def setConfig(config: GlobalConfigVo):
    GlobalConfig()._config = config
    GlobalConfig().save()
    return ResponseModel.success()

@mainConfigController.post("/reReadConfig", response_model=ResponseModel[None])
def reReadConfig():
    GlobalConfig().reRead()
    return ResponseModel.success()