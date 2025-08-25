@echo off
echo ========================================
echo    API管理系统 - C++版本构建脚本
echo ========================================
echo.

REM 检查CMake是否安装
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo 错误: 未找到CMake，请先安装CMake
    echo 下载地址: https://cmake.org/download/
    pause
    exit /b 1
)

REM 检查编译器
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo 使用Visual Studio编译器
    set COMPILER=VS
) else (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        echo 使用MinGW编译器
        set COMPILER=MinGW
    ) else (
        echo 错误: 未找到C++编译器
        echo 请安装Visual Studio或MinGW-w64
        pause
        exit /b 1
    )
)

echo.
echo 开始构建项目...

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置项目
echo 配置CMake项目...
if "%COMPILER%"=="VS" (
    cmake .. -G "Visual Studio 16 2019" -A x64
) else (
    cmake .. -G "MinGW Makefiles"
)

if %errorlevel% neq 0 (
    echo 错误: CMake配置失败
    pause
    exit /b 1
)

REM 编译项目
echo 编译项目...
if "%COMPILER%"=="VS" (
    cmake --build . --config Release
) else (
    cmake --build .
)

if %errorlevel% neq 0 (
    echo 错误: 编译失败
    pause
    exit /b 1
)

echo.
echo ========================================
echo           构建完成！
echo ========================================
echo.
echo 可执行文件位置: build\api_manager.exe
echo.
echo 运行方法:
echo   1. 直接运行: api_manager.exe
echo   2. 指定配置: api_manager.exe --config config.json
echo.
echo 按任意键退出...
pause >nul

