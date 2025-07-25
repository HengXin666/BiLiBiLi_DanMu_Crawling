# py
## 一、环境安装

> [!TIP]
> 简单的使用说明.

1. 使用 UV 管理 (下载UV)

```sh
# On macOS and Linux.
curl -LsSf https://astral.sh/uv/install.sh | sh

# On Windows.
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

# With pip.
pip install uv
```

2. 创建环境

```sh
cd py # 从项目根目录, 进入到 py 目录
uv init
```

3. 同步依赖

```sh
uv sync
```

4. 启动

```sh
uv run main.py
```