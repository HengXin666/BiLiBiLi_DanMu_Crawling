from fastapi import APIRouter
from ..utils.basePath import BasePath
from pydantic import BaseModel

from ..pojo.vo.ResponseModel import ResponseModel
from ..fileUtils.jsonConfig import GlobalConfig, MainConfig

mainConfigController = APIRouter(prefix="/mainConfig")


class GlobalConfigVo(BaseModel, MainConfig):
    pass


@mainConfigController.get("/getConfig", response_model=ResponseModel[dict])
def getConfig():
    """获取全局配置

    Returns:
        _type_: 全局配置
    """
    return ResponseModel.success(GlobalConfig().get())


@mainConfigController.post("/setConfig", response_class=ResponseModel[None])
def setConfig(config: GlobalConfigVo):
    """设置全局配置

    Args:
        config (GlobalConfigVo): 全局配置

    Returns:
        _type_: ok
    """
    GlobalConfig()._config = config
    GlobalConfig().save()
    return ResponseModel.success()


@mainConfigController.post("/reReadConfig", response_model=ResponseModel[None])
def reReadConfig():
    """从文件系统重新读取配置到内存

    Returns:
        _type_: ok
    """
    GlobalConfig().reRead()
    return ResponseModel.success()


@mainConfigController.get("/getConfigPath", response_model=ResponseModel[str])
def getConfigPath():
    """获取全局配置文件路径

    Returns:
        _type_: 全局配置文件路径
    """
    path = BasePath.relativePath()
    return ResponseModel.success(str(path))
