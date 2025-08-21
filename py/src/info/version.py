# -*- coding: utf-8 -*-

"""
MIT License

Copyright (c) 2022 Heng_Xin

本文件为版本定义
"""

import tomllib

from src.utils.basePath import BasePath


def _read_pyproject_toml():
    """
    读取并解析 pyproject.toml 文件

    Args:
        file_path (str): pyproject.toml 文件的路径

    Returns:
        dict: 解析后的 TOML 内容
    """
    file_path = BasePath.getPyProjectPath()

    try:
        with open(file_path, mode="rb") as fp:
            # 使用 rb (二进制读取模式) 来避免编码问题
            toml_dict = tomllib.load(fp)
            return toml_dict
    except FileNotFoundError:
        print(f"错误: 找不到文件 '{file_path}'")
        return None
    except Exception as e:
        print(f"解析 TOML 文件时出错: {str(e)}")
        return None


def get_version():
    """
    获取当前版本号

    Returns:
        str: 当前版本号
    """
    toml_dict = _read_pyproject_toml()

    if not toml_dict:
        print("无法读取 pyproject.toml 文件")
        return None
    if "project" not in toml_dict:
        print("pyproject.toml 文件缺少 'project' 部分")
        return None

    project = toml_dict["project"]
    return project.get("version", None)
