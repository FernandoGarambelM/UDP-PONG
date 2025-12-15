@echo off
chcp 65001 > nul
cls

echo ========================================
echo Instalando dependencias del servidor FTP
echo ========================================
echo.

python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python no esta instalado o no esta en el PATH
    echo Por favor instala Python desde https://www.python.org/
    pause
    exit /b 1
)

echo Instalando pyftpdlib...
pip install -r requirements.txt

if errorlevel 1 (
    echo.
    echo ERROR: No se pudieron instalar las dependencias
    pause
    exit /b 1
)

echo.
echo ========================================
echo Iniciando servidor FTP...
echo ========================================
echo.

python src\server\ftp_server.py

pause
