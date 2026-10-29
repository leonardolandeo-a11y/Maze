@echo off
setlocal
cd /d "%~dp0"
set "MAP_DIR=%LOCALAPPDATA%\CircuitoEscape\generated-maps"
for %%F in (mapa_facil.txt mapa_medio.txt mapa_dificil.txt) do (
    if not exist "%MAP_DIR%\%%F" (
        echo Missing generated map: "%MAP_DIR%\%%F"
        echo Use "Generar mapas" from the Start menu before playing generated maps.
        pause
        exit /b 1
    )
)
"bin\escape_circuit.exe" --maps "%MAP_DIR%"
if errorlevel 1 (
    echo.
    echo Game finished with an error.
    pause
    exit /b 1
)
endlocal
