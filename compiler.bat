@echo off
setlocal

:: ИСПОЛЬЗУЕМ ТВОЙ ПРОВЕРЕННЫЙ ПУТЬ
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\Tools\VsDevCmd.bat"

if not exist "%VS_PATH%" (
    echo [ERROR] Bat-fayl ne nayden po putu:
    echo "%VS_PATH%"
    echo Prover, chto u tebya deystvitelno VS versii 18.
    pause
    exit /b
)

echo [INFO] Inicializaciya okruzheniya VS (arch=x86)...
:: Инициализируем среду (используем x86, так как чит 32-битный)
call "%VS_PATH%" -arch=x86 -host_arch=x64

echo [INFO] Zapusk MSBuild...
:: Твоя команда с флагом лога
msbuild neverlose.sln /p:Configuration=Release /p:Platform=x86 /p:PlatformToolset=v145 -fl -flp:logfile=compile_report.txt;errorsonly

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [FAIL] Est' oshibki! Smotri compile_report.txt
) else (
    echo.
    echo [SUCCESS] Vse skompilirovalos!
)

pause