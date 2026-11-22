@echo off
setlocal
cd /d "%~dp0"

set "MINGW_BIN=C:\Program Files\CodeBlocks\MinGW\bin"
set "EXE=LEGATECH_Core\bin\Release\LEGATECH_Core.exe"
set "VERSAO_CATALOGO_TUSS=202501"
set "TUSS22=data\reference\tuss22_202501.csv"
set "TUSS63=data\reference\tuss63_202501.csv"
set "DB=data\sample\entrada_sqlite_demo.db"

if not exist "%EXE%" goto missing_exe
if not exist "%TUSS22%" goto missing_reference
if not exist "%TUSS63%" goto missing_reference
if not exist "%MINGW_BIN%\libgcc_s_seh-1.dll" goto missing_runtime
if not exist "%MINGW_BIN%\libstdc++-6.dll" goto missing_runtime

set "PATH=%MINGW_BIN%;%PATH%"
if not exist "logs" mkdir "logs"

echo.
echo LEGATECH Core - CABS002 - execucao controlada
echo 1 - CSV de amostra (10 registros)
echo 2 - CSV de validacao (8 registros)
echo 3 - SQLite de amostra (10 registros)
choice /c 123 /n /m "Selecione 1, 2 ou 3: "
if errorlevel 3 goto select_sqlite
if errorlevel 2 goto select_validation
if errorlevel 1 goto select_sample

:select_sample
set "MODE=csv_amostra"
set "INPUT=data\sample\PR_202501_AMB_DET_sample.csv"
goto setup_run

:select_validation
set "MODE=csv_validacao"
set "INPUT=data\sample\PR_202501_AMB_DET_validacao.csv"
goto setup_run

:select_sqlite
set "MODE=sqlite_amostra"
set "INPUT=%DB%"
goto setup_run

:setup_run
if not exist "%INPUT%" if /i not "%MODE%"=="sqlite_amostra" goto missing_input
for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd_HHmmss"') do set "TS=%%i"
if not defined TS goto timestamp_error
set "OUT=data\sample\saida_demo_%MODE%_%TS%"
set "LOG=logs\execucao_demo_%MODE%_%TS%.log"

(
  echo LEGATECH Core - CABS002
  echo Data/hora: %DATE% %TIME%
  echo Modo: %MODE%
  echo Entrada: %INPUT%
  echo Versao dos catalogos TUSS: %VERSAO_CATALOGO_TUSS%
  echo Catalogo TUSS 22: %TUSS22%
  echo Catalogo TUSS 63: %TUSS63%
  echo Saida: %OUT%
  echo Commit:
  git rev-parse --short HEAD
  echo.
) > "%LOG%" 2>&1

if /i "%MODE%"=="sqlite_amostra" goto run_sqlite
"%EXE%" "%INPUT%" "%TUSS22%" "%TUSS63%" --catalogo-tuss "%VERSAO_CATALOGO_TUSS%" --export "%OUT%" >> "%LOG%" 2>&1
set "RC=%ERRORLEVEL%"
goto show_result

:run_sqlite
py preparar_sqlite_demo.py >> "%LOG%" 2>&1
if errorlevel 1 goto preparation_error
"%EXE%" --sqlite "%DB%" 202501 "%TUSS22%" "%TUSS63%" --catalogo-tuss "%VERSAO_CATALOGO_TUSS%" --export "%OUT%" >> "%LOG%" 2>&1
set "RC=%ERRORLEVEL%"

:show_result
echo Exit code: %RC% >> "%LOG%"
echo.
type "%LOG%"
echo.
echo Log: %LOG%
echo Saida: %OUT%
pause
exit /b %RC%

:preparation_error
echo Preparacao da base SQLite falhou. Consulte o log: %LOG%
type "%LOG%"
pause
exit /b 1

:missing_exe
echo Executavel nao encontrado: %EXE%
goto fail

:missing_reference
echo Catalogo TUSS 22 ou 63 nao encontrado em data\reference.
goto fail

:missing_runtime
echo DLLs do MinGW nao encontradas em: %MINGW_BIN%
goto fail

:missing_input
echo Arquivo de entrada nao encontrado: %INPUT%
goto fail

:timestamp_error
echo Nao foi possivel gerar o identificador de data/hora.
goto fail

:fail
pause
exit /b 1
