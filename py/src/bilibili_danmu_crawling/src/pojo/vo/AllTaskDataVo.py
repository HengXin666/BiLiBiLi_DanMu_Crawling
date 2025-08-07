from typing import List
from pydantic import BaseModel

from ...fileUtils.jsonConfig import TaskConfig


class AllTaskDataVo(BaseModel):
    # 标题
    mainTitle: str
    # 子任务
    tasks: List[TaskConfig]
