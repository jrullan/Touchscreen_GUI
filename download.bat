@echo off
setlocal

set CLI="C:\Program Files\Arduino CLI\arduino-cli.exe"
set FQBN=esp8266:esp8266:d1_mini
set PORT=COM5
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
