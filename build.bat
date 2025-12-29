@echo off
setlocal

echo =========================================
echo   Vectoria Build Script
echo =========================================

REM MSVC paths
set "MSVC_VER=14.44.35207"
set "MSVC_ROOT=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\%MSVC_VER%"

REM Windows SDK paths
set "WINSDK_VER=10.0.22621.0"
set "WINSDK_ROOT=C:\Program Files (x86)\Windows Kits\10"

REM Add to PATH
set "PATH=%MSVC_ROOT%\bin\Hostx64\x64;%WINSDK_ROOT%\bin\%WINSDK_VER%\x64;%PATH%"

REM Set INCLUDE
set "INCLUDE=%MSVC_ROOT%\include;%WINSDK_ROOT%\Include\%WINSDK_VER%\ucrt;%WINSDK_ROOT%\Include\%WINSDK_VER%\um;%WINSDK_ROOT%\Include\%WINSDK_VER%\shared"

REM Set LIB (using onecore libs)
set "LIB=%MSVC_ROOT%\lib\onecore\x64;%WINSDK_ROOT%\Lib\%WINSDK_VER%\ucrt\x64;%WINSDK_ROOT%\Lib\%WINSDK_VER%\um\x64"

REM Add ninja to path
set "NINJA_PATH=%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
set "PATH=%NINJA_PATH%;%PATH%"

echo Compiler: cl.exe
echo INCLUDE: %INCLUDE%
echo LIB: %LIB%

cl.exe 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: cl.exe not found!
    pause
    exit /b 1
)

REM Navigate to build directory
cd /d "C:\MyProj\Vectoria"
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring CMake...
"C:\Program Files\CMake\bin\cmake.exe" .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build
echo.
echo Building...
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo =========================================
echo   Build Complete!
echo =========================================
echo.
echo Run: build\Vectoria.exe

endlocal
