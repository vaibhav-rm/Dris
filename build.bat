@echo off
echo Compiling Dris for Windows...

REM Check if gcc is available
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: GCC not found. Please install MinGW and ensure it is in your PATH.
    pause
    exit /b 1
)

REM Compile command
REM Assumes SDL2 is set up in compiler search paths or flags need to be adjusted by user
gcc dris.c -o dris.exe -lmingw32 -lSDL2main -lSDL2 -lm

if %errorlevel% neq 0 (
    echo Compilation failed!
    echo Ensure SDL2 development libraries are installed and reachable by GCC.
    pause
    exit /b 1
)

echo Compilation successful! dris.exe created.
pause
