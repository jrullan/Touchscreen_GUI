@echo off
setlocal

set CLI="C:\Program Files\Arduino CLI\arduino-cli.exe"
set FQBN=esp8266:esp8266:d1_mini
set LIBS=--libraries "c:/Users/artic/Documents/Development" --libraries "c:/Users/artic/Documents/Arduino/libraries"

if "%~1"=="" (
    echo Usage: download.bat ^<SketchName^>
    echo Example: download.bat Sample_Button
    exit /b 1
)

set SKETCH=examples\%~1\%~1.ino

if not exist "%SKETCH%" (
    echo ERROR: %SKETCH% not found.
    exit /b 1
)

echo === Detecting board ===
set PORT=

rem Try 1: arduino-cli recognized board (FQBN match)
for /f "tokens=1" %%p in ('%CLI% board list 2^>nul ^| findstr /i "esp8266"') do (
    set PORT=%%p
)
if not "%PORT%"=="" (
    echo Found ESP8266 on %PORT% via arduino-cli
    goto compile
)

rem Try 2: CP210x USB-to-UART (driver name contains COM port)
for /f "tokens=*" %%n in ('powershell -NoProfile -Command "Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match 'CP210.*\(COM\d+\)' } | ForEach-Object { if ($_.Name -match '\(COM(\d+)\)') { 'COM' + $matches[1] } }" 2^>nul') do (
    set PORT=%%n
)
if not "%PORT%"=="" (
    echo Found CP210x on %PORT%
    goto compile
)

rem Try 3: CH340 USB-to-UART
for /f "tokens=*" %%n in ('powershell -NoProfile -Command "Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match 'CH340.*\(COM\d+\)' } | ForEach-Object { if ($_.Name -match '\(COM(\d+)\)') { 'COM' + $matches[1] } }" 2^>nul') do (
    set PORT=%%n
)
if not "%PORT%"=="" (
    echo Found CH340 on %PORT%
    goto compile
)

echo ERROR: No board detected.
echo.
echo arduino-cli sees:
%CLI% board list 2>nul
echo.
echo USB serial devices:
powershell -NoProfile -Command "Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match 'CP210|CH340|USB.*Serial|FTDI' } | Select-Object -ExpandProperty Name"
exit /b 1

:compile
echo === Compiling %~1 ===
%CLI% compile --fqbn %FQBN% %LIBS% "%SKETCH%"
if errorlevel 1 (
    echo === Compile FAILED ===
    exit /b 1
)

echo === Uploading to %PORT% ===
%CLI% upload --fqbn %FQBN% --port %PORT% "%SKETCH%"
if errorlevel 1 (
    echo === Upload FAILED ===
    exit /b 1
)

echo === Done ===
