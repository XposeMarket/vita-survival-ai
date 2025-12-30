@echo off
REM Survival AI - Automated Build Script for Windows
REM This handles all the path nonsense for you

echo ================================================
echo Survival AI - Automated Build
echo ================================================
echo.

REM Set VitaSDK path
set VITASDK=C:\vitasdk
set PATH=%PATH%;C:\vitasdk\bin;C:\Program Files\CMake\bin

echo Checking VitaSDK...
if not exist "%VITASDK%\bin\arm-vita-eabi-gcc.exe" (
    echo ERROR: VitaSDK not found at C:\vitasdk
    echo Please copy your VitaSDK to C:\vitasdk first
    pause
    exit /b 1
)

echo Checking CMake...
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found
    echo Please install CMake from cmake.org
    pause
    exit /b 1
)

echo.
echo Cleaning old build...
if exist build rmdir /s /q build
mkdir build
cd build

echo.
echo Running CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM="%VITASDK%\bin\make-3.81.exe"
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    cd ..
    pause
    exit /b 1
)

echo.
echo Building VPK...
cmake --build .
if errorlevel 1 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo.
echo ================================================
echo BUILD SUCCESSFUL!
echo ================================================
echo.
echo VPK Location: build\SurvivalAI.vpk
echo.
dir *.vpk
echo.
pause
