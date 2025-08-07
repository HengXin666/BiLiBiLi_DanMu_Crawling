import shutil
from pathlib import Path

def deleteFolder(folder: Path) -> None:
    """递归删除该文件夹

    Args:
        folder (Path): 文件夹路径

    Raises:
        ValueError: 不是文件夹
    """
    if not folder.exists():
        return
    if not folder.is_dir():
        raise ValueError(f"{folder} 不是文件夹")
    
    shutil.rmtree(folder)

def deleteIfParentEmpty(path: Path) -> None:
    """判断给定路径的父目录, 如果为空则删除

    Args:
        path (Path): 文件夹路径
    """
    parent = path.parent
    if not parent.exists():
        return  # 父目录都没了, 直接返回

    # 判断父目录是否为空
    if not any(parent.iterdir()):
        parent.rmdir()  # 只删除空目录