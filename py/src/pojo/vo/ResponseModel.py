from typing import Generic, TypeVar, Optional
from fastapi.responses import JSONResponse
from pydantic import BaseModel

T = TypeVar("T")

class ResponseModel(BaseModel, Generic[T]):
    code: int
    msg: str
    data: Optional[T] = None

    @staticmethod
    def success(data=None, msg="success") -> JSONResponse:
        return JSONResponse(content=ResponseModel(code=0, msg=msg, data=data).model_dump())

    @staticmethod
    def error(code=500, msg="error") -> JSONResponse:
        return JSONResponse(content=ResponseModel(code=code, msg=msg, data=None).model_dump())
