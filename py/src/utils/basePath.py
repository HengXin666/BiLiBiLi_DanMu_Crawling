import sys
from pathlib import Path

class BasePath:
    @staticmethod
    def _getAppRoot() -> Path:
        """
        获取程序所在目录：
        - 脚本运行时，返回 __file__ 的目录
        - PyInstaller 二进制运行时，返回 _MEIPASS 临时目录
        """
        if getattr(sys, 'frozen', False):
            # PyInstaller 模式
            return Path(sys._MEIPASS) # type: ignore
        else:
            return Path(__file__).resolve().parent

    @staticmethod
    def relativePath(relative: str | Path) -> Path:
        """
        相对路径，基于程序所在目录，而不是 cwd
        """
        return BasePath._getAppRoot() / relative