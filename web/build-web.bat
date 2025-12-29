@echo off
REM Vectoria Web Build Script for Windows
REM Requires Emscripten SDK to be installed and activated

setlocal enabledelayedexpansion

echo =========================================
echo   Vectoria Web Build (Emscripten)
echo =========================================

REM Get script directory
set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."
set "BUILD_DIR=%PROJECT_DIR%\build-web"
set "OUTPUT_DIR=%PROJECT_DIR%\web\dist"

REM Check if Emscripten is available
where emcc >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: Emscripten not found!
    echo.
    echo Please install and activate Emscripten SDK:
    echo   git clone https://github.com/emscripten-core/emsdk.git
    echo   cd emsdk
    echo   emsdk install latest
    echo   emsdk activate latest
    echo   emsdk_env.bat
    exit /b 1
)

echo Emscripten found.

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure with Emscripten
echo.
echo Configuring CMake with Emscripten...
call emcmake cmake "%PROJECT_DIR%" -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build
echo.
echo Building...
call emmake mingw32-make -j4
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

REM Create output directory and copy files
echo.
echo Copying output files...
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
copy "%BUILD_DIR%\Vectoria.html" "%OUTPUT_DIR%\index.html" >nul
copy "%BUILD_DIR%\Vectoria.js" "%OUTPUT_DIR%\" >nul
copy "%BUILD_DIR%\Vectoria.wasm" "%OUTPUT_DIR%\" >nul
copy "%BUILD_DIR%\Vectoria.data" "%OUTPUT_DIR%\" >nul 2>nul

echo.
echo =========================================
echo   Build Complete!
echo =========================================
echo.
echo Output files in: %OUTPUT_DIR%
echo.
echo To test locally, run:
echo   cd %OUTPUT_DIR%
echo   python -m http.server 8080
echo.
echo Then open: http://localhost:8080
echo.

endlocal
