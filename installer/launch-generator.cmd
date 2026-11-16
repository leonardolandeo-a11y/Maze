@echo off
setlocal
cd /d "%~dp0"
set "OUTPUT_DIR=%LOCALAPPDATA%\CircuitoEscape\generated-maps"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
"bin\map_generator.exe" --interactive --output "%OUTPUT_DIR%"
if errorlevel 1 (
    echo.
    echo Map generation failed. Check the error above; no new set of maps is ready.
    pause
    exit /b 1
)
echo.
echo Generated maps directory: "%OUTPUT_DIR%"
pause
endlocal
