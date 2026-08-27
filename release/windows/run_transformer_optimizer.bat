@echo off
setlocal
cd /d "%~dp0"
if exist bin\transformer_optimizer.exe (
    start "Transformer Optimizer" "bin\transformer_optimizer.exe"
) else (
    echo.
    echo No se encontro transformer_optimizer.exe.
    echo Ejecuta build_release.ps1 primero o compila el proyecto con MinGW/MSYS2.
    echo.
    pause
)
