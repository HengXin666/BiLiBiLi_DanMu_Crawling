from typing import Generic, TypeVar, Optional
from fastapi.responses import JSONResponse
from pydantic.generics import GenericModel

T = TypeVar("T")

class ResponseModel(GenericModel, Generic[T]):
    code: int
    msg: str
    data: Optional[T] = None

    @staticmethod
    def success(data=None, msg="success") -> JSONResponse:
        return JSONResponse(content=ResponseModel(code=0, msg=msg, data=data).dict())

    @staticmethod
    def error(code=500, msg="error") -> JSONResponse:
        return JSONResponse(content=ResponseModel(code=code, msg=msg, data=None).dict())
