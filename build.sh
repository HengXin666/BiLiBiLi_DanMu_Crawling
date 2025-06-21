#!/bin/bash
set -e

# 变量定义
APP_NAME=BiLiBiLi_DanMu_Crawling
BUILD_DIR=tmp-hx-build
APPDIR=$BUILD_DIR/$APP_NAME.AppDir
VENV_DIR=$BUILD_DIR/venv
DIST_DIR=$BUILD_DIR/dist
PYINSTALLER_SPEC=$BUILD_DIR/main.spec

#!/bin/bash
set -e

APP_NAME=BiLiBiLi_DanMu_Crawling
BUILD_DIR=tmp-hx-build
APPDIR=$BUILD_DIR/$APP_NAME.AppDir
APPIMAGE_TOOL=$BUILD_DIR/appimagetool-x86_64.AppImage

# 清理旧构建缓存
echo "[1/6] 清理构建目录..."
rm -rf "$BUILD_DIR"
mkdir -p "$APPDIR/usr/bin"

# ===[ A. 配置代理（可选）]===
# 如果你需要代理，请取消下面的注释并填写代理地址：
export https_proxy=http://172.27.160.1:2334
export http_proxy=http://172.27.160.1:2334

# ===[ B. 提前下载 appimagetool（带超时 + 失败提示）]===
echo "[2/6] 准备 AppImage 工具..."
mkdir -p "$BUILD_DIR"
if [ ! -f "$APPIMAGE_TOOL" ]; then
    echo "正在尝试下载 appimagetool（30秒超时）..."

    if ! wget --timeout=30 --tries=2 -O "$APPIMAGE_TOOL" \
        https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage; then
        echo
        echo "❌ 下载 appimagetool 失败！你可以手动下载："
        echo "   👉 https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage"
        echo
        echo "📦 下载完成后请将它保存为："
        echo "   $APPIMAGE_TOOL"
        echo
        echo "🔁 手动下载命令示例："
        echo "   wget -O \"$APPIMAGE_TOOL\" https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage"
        echo "   curl -L -o \"$APPIMAGE_TOOL\" https://github.com/AppImage/AppImageKit/releases/latest/download/appimagetool-x86_64.AppImage"
        echo
        echo "‼️ 当前打包流程无法继续，请先准备好 appimagetool 工具。"
        exit 1
    fi

    chmod +x "$APPIMAGE_TOOL"
fi

echo "[3/6] 安装 Python 3.12 及相关工具..."
sudo add-apt-repository -y ppa:deadsnakes/ppa
sudo apt update
sudo apt install -y python3.12 python3.12-venv python3.12-dev python3-pip libfuse2 wget
sudo apt install -y python3.12-tk

echo "[3/6] 创建 Python 3.12 虚拟环境..."
python3.12 -m venv "$VENV_DIR"
source "$VENV_DIR/bin/activate"

# 安装依赖
echo "[4/6] 安装依赖..."
pip install --upgrade pip
pip install pyinstaller
pip install -r requirements.txt

# 使用 PyInstaller 打包
echo "[5/6] 使用 PyInstaller 打包程序..."
pyinstaller --name "$APP_NAME" --onefile main.py --distpath "$DIST_DIR" --specpath "$BUILD_DIR" --workpath "$BUILD_DIR/build"

# 拷贝可执行文件到 AppDir
cp "$DIST_DIR/$APP_NAME" "$APPDIR/usr/bin/"

# 创建 AppRun 启动器
cat > "$APPDIR/AppRun" <<EOF
#!/bin/bash
HERE=\$(dirname "\$(readlink -f "\$0")")
exec "\$HERE/usr/bin/$APP_NAME" "\$@"
EOF
chmod +x "$APPDIR/AppRun"

# 创建 .desktop 文件
cat > "$APPDIR/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=$APP_NAME
Icon=app
Categories=Utility;
EOF

# 生成图标
cp src/ico/app.png "$APPDIR/"

# 生成 AppImage
echo "[6/6] 生成 AppImage..."
cd "$BUILD_DIR"

echo "[调试] 当前目录：$(pwd)"
echo "[调试] 目录下文件列表："
ls -l

echo "[调试] 尝试执行：$APPIMAGE_TOOL $APP_NAME.AppDir"
ARCH=x86_64 ./appimagetool-x86_64.AppImage "$APP_NAME.AppDir"


# 移动成品回项目目录
mv "$APP_NAME"-x86_64.AppImage ../
cd ..

echo
echo "✅ 打包完成：$APP_NAME-x86_64.AppImage"
echo "📁 构建缓存已存于：$BUILD_DIR"
