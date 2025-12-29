@echo off
REM Build script for Gravity Paint (Windows/MinGW)

set PATH=C:\msys64\mingw64\bin;%PATH%

if not exist build-mingw mkdir build-mingw
cd build-mingw

cmake -G "Ninja" ..
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

cmake --build . -j4
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build successful! Run GravityPaint.exe to play.
pause
