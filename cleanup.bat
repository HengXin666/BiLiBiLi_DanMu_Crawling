@echo off
REM 删除 build 文件夹
if exist build (
    rmdir /s /q build
    echo Deleted build folder.
) else (
    echo build folder does not exist.
)

REM 删除 dist 文件夹
if exist dist (
    rmdir /s /q dist
    echo Deleted dist folder.
) else (
    echo dist folder does not exist.
)

pause