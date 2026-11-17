@echo off
title TransitX Presentation Launcher
color 0A
echo ======================================================================
echo           TransitX - Real-Time Public Bus Tracking System
echo                     Class Presentation Launcher
echo ======================================================================
echo.

:: Check Node.js
where node >nul 2>nul
if %errorlevel% neq 0 (
    color 0C
    echo [ERROR] Node.js is not installed on this computer!
    echo Please download and install Node.js (LTS version) from:
    echo    https://nodejs.org/
    echo.
    echo After installing Node.js, run this file again.
    echo ======================================================================
    pause
    exit /b 1
)

:: Compile & Start Pure C Backend (if GCC exists)
echo [1/3] Checking C Backend...
cd /d "%~dp0backend_c"
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Compiling Pure C Backend with GCC...
    gcc -std=c11 -O2 src/*.c -Iinclude -o transitx_c.exe -lws2_32 -lm 2>nul
    if exist transitx_c.exe (
        echo Starting Pure C Backend on port 8080...
        start "TransitX C Backend Server" cmd /k "transitx_c.exe"
    )
) else (
    echo Note: GCC not detected. The Passenger app will use its built-in
    echo real-time telemetry engine for the presentation.
)

:: Install Frontend Dependencies if needed
echo.
echo [2/3] Preparing Passenger Web Application...
cd /d "%~dp0frontend"
if not exist node_modules (
    echo Installing dependencies (this only happens on the first run, ~25 sec)...
    call npm install
)

:: Launch Browser & Dev Server
echo.
echo [3/3] Launching TransitX Dashboard...
echo Opening http://localhost:5173 in your default browser...
start "" http://localhost:5173

echo.
echo ======================================================================
echo   TransitX is running!
echo   Dashboard:  http://localhost:5173
echo   Press Ctrl+C to stop the presentation server.
echo ======================================================================
echo.
call npm run dev
