import ast
import os
import platform
import sys
from pathlib import Path


class BasePath:
    @staticmethod
    def _getSysDefaultPath() -> Path:
        """
        获取系统默认路径
        """
        system = platform.system()
        if system == "Windows":
            base = os.getenv("LOCALAPPDATA", os.getenv("APPDATA"))
        elif system == "Darwin":  # macOS
            base = os.path.expanduser("~/Library/Application Support")
        else:  # Linux/Unix
            base = os.getenv("XDG_CONFIG_HOME", os.path.expanduser("~/.config"))
        path = Path.joinpath(Path(base), "bilibili_danmu_crawling")
        Path.mkdir(path, exist_ok=True)
        return path

    @staticmethod
    def _getAppRoot() -> Path:
        """
        获取程序根目录（开发环境）或临时目录（PyInstaller）
        """
        conf_path = os.getenv("BILIBILI_DANMU_CRAWLING_CONFIG_PATH")
        if conf_path is not None:
            conf_path = Path(conf_path)
            if conf_path.is_file():
                raise ValueError(f"config 路径只接受文件夹；缺收到文件：{conf_path}")
            else:
                return conf_path
        elif ast.literal_eval(os.getenv("BILIBILI_DANMU_CRAWLING_DEV")):
            # 开发环境：假设 py/ 是源码根目录，返回项目根路径
            return Path(__file__).resolve().parent.parent.parent
        elif getattr(sys, "frozen", False):
            # PyInstaller 模式
            return Path(sys.executable).parent  # type: ignore
        else:
            return BasePath._getSysDefaultPath()

    @staticmethod
    def getPyProjectPath() -> Path:
        return Path(__file__).resolve().parent.parent.parent / "pyproject.toml"

    @staticmethod
    def relativePath(relative: str | Path | None = None) -> Path:
        if relative is None:
            return BasePath._getAppRoot().resolve()
        else:
            relative = Path(relative)
            if relative.is_absolute():
                raise ValueError(f"relativePath 只能传相对路径; 却收到: {relative}")
            path = BasePath._getAppRoot() / relative
            return path.resolve()
