import os
from pathlib import Path
import re
from typing import List

from ..pojo.vo.AllTaskDataVo import AllTaskDataVo
from ..fileUtils.jsonConfig import DATA_PATH, TaskConfig, TaskConfigManager
from ..utils.basePath import BasePath

def getAllTaskData():
    rootPath = BasePath.relativePath(f"{DATA_PATH}/")
    
    res: List[AllTaskDataVo] = []

    for dirName in os.listdir(rootPath):
        dirPath = os.path.join(rootPath, dirName)
        if not os.path.isdir(dirPath):
            continue
        tasks: List[TaskConfig] = []

        for subDirName in os.listdir(dirPath):
            subDirPath = os.path.join(dirPath, subDirName)
            if not os.path.isdir(subDirPath):
                continue
            for fileName in os.listdir(subDirPath):
                # 匹配形如 {cid}_config.json 的文件
                match = re.match(r"(\d+)_config\.json", fileName)
                if match:
                    tasks.append(TaskConfigManager(Path(
                        f"{subDirPath}/{fileName}")).load())

        res.append(AllTaskDataVo(
            mainTitle = dirName,
            tasks = tasks
        ))
    
    return res