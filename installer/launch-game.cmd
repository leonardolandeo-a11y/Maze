@echo off
setlocal
cd /d "%~dp0"
"bin\escape_circuit.exe" --maps "assets\maps"
if errorlevel 1 (
    echo.
    echo Game finished with an error.
    pause
)
endlocal
