@echo off
setlocal enabledelayedexpansion
title Lenia Build System

:: ============================================================
::  Lenia - Generalized Artificial Life
::  Windows Build Script  (MinGW / MSYS2 UCRT64)
::
::  Usage:  build_windows.bat [options]
::    --no-pause    Skip the pause at end
::    --run         Launch Lenia.exe after a successful build
::    --clean       Delete obj/ before building
::    --release     Build with -O3 (default is -O2)
:: ============================================================

:: ---- Parse arguments ---------------------------------------------
set "NO_PAUSE=0"
set "AUTO_RUN=0"
set "DO_CLEAN=0"
set "OPT_LEVEL=-O2"
for %%a in (%*) do (
    if /i "%%a"=="--no-pause" set "NO_PAUSE=1"
    if /i "%%a"=="--run"      set "AUTO_RUN=1"
    if /i "%%a"=="--clean"    set "DO_CLEAN=1"
    if /i "%%a"=="--release"  set "OPT_LEVEL=-O3"
)

:: ---- Timestamp ---------------------------------------------------
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value 2^>nul') do set "DT=%%I"
set "TIMESTAMP=%DT:~0,4%-%DT:~4,2%-%DT:~6,2% %DT:~8,2%:%DT:~10,2%:%DT:~12,2%"

:: ---- Record build start time -------------------------------------
set "T_START=%time%"

:: ---- Toolchain ---------------------------------------------------
SET "PATH=C:\msys64\ucrt64\bin;%PATH%"
set "CXX=g++"
set "CC=gcc"
set "CXXFLAGS=-std=c++20 !OPT_LEVEL! -Wall -Wextra -fpermissive"

:: ---- Paths -------------------------------------------------------
set "INCLUDES=-Isrc -Ilibs\imgui -Ilibs\imgui\backends -Ilibs\glad\include -Ilibs\glm -Ilibs\glfw\include"
set "LDFLAGS=-Llibs\glfw\lib"
set "LDLIBS=-lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32"
set "STATIC_RT=-static -mwindows"

set "SRC_DIR=src"
set "OBJ_DIR=obj"
set "BIN_DIR=bin"
set "LOG_DIR=log"
set "TARGET=%BIN_DIR%\Lenia.exe"

:: ---- Counters for summary ----------------------------------------
set /a "TOTAL=0"
set /a "OK=0"
set /a "FAIL=0"
set "HAD_ERROR=0"

:: ==================================================================
echo.
echo  ========================================================
echo   Lenia Build System               %TIMESTAMP%
echo   Options: opt=%OPT_LEVEL%  clean=%DO_CLEAN%  run=%AUTO_RUN%
echo  ========================================================
echo.

:: ---- Optional clean ----------------------------------------------
if "%DO_CLEAN%"=="1" (
    echo  [*] Cleaning obj\ ...
    if exist "%OBJ_DIR%" rmdir /s /q "%OBJ_DIR%"
    echo         Done.
    echo.
)

:: ---- Verify toolchain --------------------------------------------
echo  [1/7] Checking toolchain...
where g++ >nul 2>&1
if errorlevel 1 (
    echo         ERROR: g++ not found in PATH.
    echo         Make sure MSYS2 UCRT64 is installed and C:\msys64\ucrt64\bin is in PATH.
    goto :fail
)
for /f "tokens=*" %%v in ('g++ --version 2^>^&1') do (
    echo         g++ : %%v
    goto :done_ver
)
:done_ver

:: ---- Verify libraries exist --------------------------------------
echo  [2/7] Verifying libraries...
set "MISSING="
if not exist "libs\glad\include\glad\glad.h"              set "MISSING=!MISSING! GLAD"
if not exist "libs\glad\src\glad.c"                       set "MISSING=!MISSING! GLAD(src)"
if not exist "libs\glfw\include\GLFW\glfw3.h"             set "MISSING=!MISSING! GLFW(headers)"
if not exist "libs\glfw\lib\libglfw3.a"                   set "MISSING=!MISSING! GLFW(lib)"
if not exist "libs\imgui\imgui.h"                         set "MISSING=!MISSING! ImGui"
if not exist "libs\imgui\backends\imgui_impl_glfw.cpp"    set "MISSING=!MISSING! ImGui(glfw)"
if not exist "libs\imgui\backends\imgui_impl_opengl3.cpp" set "MISSING=!MISSING! ImGui(gl3)"
if defined MISSING (
    echo         ERROR: Missing libraries:%MISSING%
    echo         Run setup_libs.ps1 or install them manually into libs\.
    goto :fail
)
echo         All libraries found.

:: ---- Verify shaders exist ----------------------------------------
echo  [3/7] Verifying shader assets...
set "SHADER_OK=1"
for %%s in (kernel_gen.comp sim_spatial.comp sim_noise.comp sim_multichannel.comp analysis.comp display.vert display.frag) do (
    if not exist "assets\shaders\%%s" (
        echo         ERROR: Missing shader assets\shaders\%%s
        set "SHADER_OK=0"
    )
)
if "%SHADER_OK%"=="0" goto :fail
echo         All 7 shaders found.

:: ---- Create directories ------------------------------------------
echo  [4/7] Preparing directories...
if not exist "%OBJ_DIR%"   mkdir "%OBJ_DIR%"
if not exist "%BIN_DIR%"   mkdir "%BIN_DIR%"
if not exist "%LOG_DIR%"   mkdir "%LOG_DIR%"
echo         obj\  bin\  log\  ready.

:: ==================================================================
::  COMPILATION
:: ==================================================================
echo  [5/7] Compiling source files...
echo.

:: ---- Application sources (src\*.cpp) -----------------------------
echo    --- Application sources ---
for %%f in (%SRC_DIR%\*.cpp) do (
    set /a "TOTAL+=1"
    echo         %%~nxf
    %CXX% %CXXFLAGS% %INCLUDES% -c "%%f" -o "%OBJ_DIR%\%%~nf.o" 2>&1
    if errorlevel 1 (
        echo         ^^!^^!^^! FAILED: %%~nxf
        set /a "FAIL+=1"
        set "HAD_ERROR=1"
    ) else (
        set /a "OK+=1"
    )
)

:: ---- Utility sources (src\Utils\*.cpp) ---------------------------
if exist "%SRC_DIR%\Utils" (
    echo    --- Utility sources ---
    for %%f in (%SRC_DIR%\Utils\*.cpp) do (
        set /a "TOTAL+=1"
        echo         %%~nxf
        %CXX% %CXXFLAGS% %INCLUDES% -c "%%f" -o "%OBJ_DIR%\Utils_%%~nf.o" 2>&1
        if errorlevel 1 (
            echo         ^^!^^!^^! FAILED: %%~nxf
            set /a "FAIL+=1"
            set "HAD_ERROR=1"
        ) else (
            set /a "OK+=1"
        )
    )
)

:: ---- GLAD (C source) --------------------------------------------
echo    --- GLAD ---
set /a "TOTAL+=1"
echo         glad.c
%CC% -O2 -Wall %INCLUDES% -c "libs\glad\src\glad.c" -o "%OBJ_DIR%\glad.o" 2>&1
if errorlevel 1 (
    echo         ^^!^^!^^! FAILED: glad.c
    set /a "FAIL+=1"
    set "HAD_ERROR=1"
) else (
    set /a "OK+=1"
)

:: ---- Dear ImGui core ---------------------------------------------
echo    --- Dear ImGui (core) ---
for %%f in (imgui imgui_draw imgui_tables imgui_widgets) do (
    set /a "TOTAL+=1"
    echo         %%f.cpp
    %CXX% %CXXFLAGS% %INCLUDES% -c "libs\imgui\%%f.cpp" -o "%OBJ_DIR%\%%f.o" 2>&1
    if errorlevel 1 (
        echo         ^^!^^!^^! FAILED: %%f.cpp
        set /a "FAIL+=1"
        set "HAD_ERROR=1"
    ) else (
        set /a "OK+=1"
    )
)

:: ---- Dear ImGui backends -----------------------------------------
echo    --- Dear ImGui (backends) ---
set /a "TOTAL+=1"
echo         imgui_impl_glfw.cpp
%CXX% %CXXFLAGS% %INCLUDES% -DGLFW_INCLUDE_NONE -c "libs\imgui\backends\imgui_impl_glfw.cpp" -o "%OBJ_DIR%\imgui_impl_glfw.o" 2>&1
if errorlevel 1 (
    echo         ^^!^^!^^! FAILED: imgui_impl_glfw.cpp
    set /a "FAIL+=1"
    set "HAD_ERROR=1"
) else (
    set /a "OK+=1"
)

set /a "TOTAL+=1"
echo         imgui_impl_opengl3.cpp  (using GLAD loader)
%CXX% %CXXFLAGS% %INCLUDES% -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM -include "glad/glad.h" -c "libs\imgui\backends\imgui_impl_opengl3.cpp" -o "%OBJ_DIR%\imgui_impl_opengl3.o" 2>&1
if errorlevel 1 (
    echo         ^^!^^!^^! FAILED: imgui_impl_opengl3.cpp
    set /a "FAIL+=1"
    set "HAD_ERROR=1"
) else (
    set /a "OK+=1"
)

:: ---- Windows Resource File (.rc) ---------------------------------
echo    --- Windows Resources ---
if exist "%SRC_DIR%\Lenia.rc" (
    set /a "TOTAL+=1"
    echo         Lenia.rc
    windres -i "%SRC_DIR%\Lenia.rc" -o "%OBJ_DIR%\Lenia_res.o" --include-dir="%SRC_DIR%" 2>&1
    if errorlevel 1 (
        echo         ^^!^^!^^! FAILED: Lenia.rc
        set /a "FAIL+=1"
        set "HAD_ERROR=1"
    ) else (
        set /a "OK+=1"
    )
)

echo.
echo         Compilation: !OK!/!TOTAL! succeeded, !FAIL! failed.
if "!HAD_ERROR!"=="1" (
    echo.
    echo         ERROR: Some source files failed to compile. See messages above.
    goto :fail
)

:: ==================================================================
::  LINKING
:: ==================================================================
echo  [6/7] Linking %TARGET%...

set "OBJ_FILES="
for %%f in (%OBJ_DIR%\*.o) do set "OBJ_FILES=!OBJ_FILES! "%%f""

%CXX% !OBJ_FILES! -o "%TARGET%" %LDFLAGS% %LDLIBS% %STATIC_RT% 2>&1
if errorlevel 1 (
    echo         ERROR: Linker failed. See messages above.
    goto :fail
)

:: Print binary size
for %%A in (%TARGET%) do (
    set /a "SIZE_KB=%%~zA / 1024"
    echo         OK  %TARGET%  ^(!SIZE_KB! KB^)
)

:: ==================================================================
::  POST-BUILD
:: ==================================================================
echo  [7/7] Copying assets...

if not exist "%BIN_DIR%\assets\shaders" mkdir "%BIN_DIR%\assets\shaders"
copy /Y "assets\shaders\*.*" "%BIN_DIR%\assets\shaders\" >nul
echo         Shaders copied to %BIN_DIR%\assets\shaders\

if exist "assets\icon.png" (
    if not exist "%BIN_DIR%\assets" mkdir "%BIN_DIR%\assets"
    copy /Y "assets\icon.png" "%BIN_DIR%\assets\" >nul
    echo         Icon PNG copied.
)

if exist "assets\icon.ico" (
    if not exist "%BIN_DIR%\assets" mkdir "%BIN_DIR%\assets"
    copy /Y "assets\icon.ico" "%BIN_DIR%\assets\" >nul
    echo         Icon ICO copied.
)

if not exist "%BIN_DIR%\Initialisation" mkdir "%BIN_DIR%\Initialisation"
if exist "Initialisation\*.npy" (
    copy /Y "Initialisation\*.npy" "%BIN_DIR%\Initialisation\" >nul
    echo         Species data copied to %BIN_DIR%\Initialisation\
)

if exist "colormap" (
    if not exist "%BIN_DIR%\colormap" mkdir "%BIN_DIR%\colormap"
    copy /Y "colormap\*.*" "%BIN_DIR%\colormap\" >nul
    echo         Colormaps copied to %BIN_DIR%\colormap\
)

:: ---- Build time --------------------------------------------------
set "T_END=%time%"
call :elapsed "%T_START%" "%T_END%" ELAPSED

:: ==================================================================
::  DONE
:: ==================================================================
echo.
echo  ========================================================
echo   BUILD SUCCESSFUL    (%ELAPSED%s)
echo.
echo   Binary:  %TARGET%  ^(!SIZE_KB! KB^)
echo.
echo   Run the application:
echo       cd bin ^&^& Lenia.exe
echo       bin\Lenia.exe
echo  ========================================================
echo.
if "%AUTO_RUN%"=="1" (
    echo  Launching Lenia...
    start "" "%TARGET%"
)
if "%NO_PAUSE%"=="0" pause
exit /b 0

:: ==================================================================
:fail
echo.
echo  ========================================================
echo   BUILD FAILED  -  Fix the errors above and try again.
echo  ========================================================
echo.
if "%NO_PAUSE%"=="0" pause
exit /b 1

:: ==================================================================
:: Subroutine: compute elapsed seconds from two %time% values
:: ==================================================================
:elapsed
setlocal
set "s=%~1"
set "e=%~2"
for /f "tokens=1-4 delims=:., " %%a in ("%s%") do (
    set /a "ss=((%%a*60+%%b)*60+%%c)*100+%%d"
)
for /f "tokens=1-4 delims=:., " %%a in ("%e%") do (
    set /a "se=((%%a*60+%%b)*60+%%c)*100+%%d"
)
set /a "diff=se-ss"
if !diff! lss 0 set /a "diff+=8640000"
set /a "secs=diff/100"
set /a "cents=diff%%100"
if !cents! lss 10 set "cents=0!cents!"
endlocal & set "%~3=%secs%.%cents%"
goto :eof