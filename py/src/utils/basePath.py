import sys
from pathlib import Path

class BasePath:
    @staticmethod
    def _getAppRoot() -> Path:
        """
        获取程序根目录（开发环境）或临时目录（PyInstaller）
        """
        if getattr(sys, 'frozen', False):
            # PyInstaller 模式
            return Path(sys._MEIPASS) # type: ignore
        else:
            # 开发环境：假设 py/ 是源码根目录，返回项目根路径
            return Path(__file__).resolve().parent.parent.parent

    @staticmethod
    def relativePath(relative: str | Path) -> Path:
        relative = Path(relative)
        if relative.is_absolute():
            raise ValueError(f"relativePath 只能传相对路径，收到: {relative}")
        path = BasePath._getAppRoot() / relative
        return path.resolve()
