@echo off

if "%~1"=="" (
    echo "Usage: psdhtml.cmd FILE.psd..."
    exit /b
)

set LAUNCHER=%~0
set DIR=%LAUNCHER%\..\
cd %DIR%

:do_psdhtml
"%~dp0libexec\psdhtml.exe" "%~1"
copy "%~dp0static\*" "%~n1_html\%~n1.files"
shift
if "%~1"=="" exit /b
goto do_psdhtml
