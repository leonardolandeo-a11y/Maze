@echo off
setlocal
cd /d "%~dp0"
set "OUTPUT_DIR=%LOCALAPPDATA%\CircuitoEscape\generated-maps"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
"bin\map_generator.exe" --rows 20 --columns 30 --seed 12345 --workers 4 --candidates 10 --initial-energy 100 --maximum-energy 100 --turn-limit 240 --extra-openings 15 --search-states 30000 --resources 5 --resource-points 10 --batteries 3 --battery-recharge 5 --traps 3 --trap-penalty 2 --trap-score-penalty 1 --output "%OUTPUT_DIR%"
echo.
echo Generated maps directory: "%OUTPUT_DIR%"
pause
endlocal
