@echo off
echo Starting the game.

REM Find cmake executable
set CMAKE_EXE=cmake
where cmake >nul 2>&1
if errorlevel 1 (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set CMAKE_EXE="C:\Program Files\CMake\bin\cmake.exe"
        goto :cmake_found
    )
    if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
        set CMAKE_EXE="C:\Program Files (x86)\CMake\bin\cmake.exe"
        goto :cmake_found
    )
    if exist "%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe" (
        set CMAKE_EXE="%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe"
        goto :cmake_found
    )
    echo CMake not found. Please install CMake or add it to your PATH.
    echo You can download CMake from: https://cmake.org/download/
    pause
    exit /b 1
)
:cmake_found

REM Find vcpkg toolchain file
set VCPKG_TOOLCHAIN=
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
        set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
        goto :vcpkg_found
    )
)
if exist "C:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
    goto :vcpkg_found
)
if exist "C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake"
    goto :vcpkg_found
)
if exist "%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%USERPROFILE%\vcpkg\scripts\buildsystems\vcpkg.cmake"
    goto :vcpkg_found
)
if exist "%LOCALAPPDATA%\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=%LOCALAPPDATA%\vcpkg\scripts\buildsystems\vcpkg.cmake"
    goto :vcpkg_found
)
if exist "D:\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set "VCPKG_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=D:\vcpkg\scripts\buildsystems\vcpkg.cmake"
    goto :vcpkg_found
)

REM vcpkg not found - check if we need to build
set NEED_BUILD=1
if exist "build\SlimesSkyTravel.exe" set NEED_BUILD=0
if exist "build\Release\SlimesSkyTravel.exe" set NEED_BUILD=0
if exist "build\Debug\SlimesSkyTravel.exe" set NEED_BUILD=0

if %NEED_BUILD%==1 (
    echo.
    echo ========================================
    echo WARNING: vcpkg not found
    echo ========================================
    echo.
    echo vcpkg toolchain file not found. This project requires vcpkg to build.
    echo.
    echo If you have a pre-built executable, it will be used.
    echo Otherwise, please install vcpkg:
    echo   1. Open PowerShell as Administrator
    echo   2. Run: git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    echo   3. Run: cd C:\vcpkg
    echo   4. Run: .\bootstrap-vcpkg.bat
    echo   5. Run: .\vcpkg install glfw3:x64-windows glm:x64-windows nlohmann-json:x64-windows sdl2-mixer:x64-windows
    echo   6. Set environment variable: setx VCPKG_ROOT "C:\vcpkg"
    echo.
    echo Alternatively, you can install vcpkg to one of these locations:
    echo   - C:\vcpkg
    echo   - C:\tools\vcpkg
    echo   - %USERPROFILE%\vcpkg
    echo   - %LOCALAPPDATA%\vcpkg
    echo   - D:\vcpkg
    echo.
    echo Or set the VCPKG_ROOT environment variable to point to your vcpkg installation.
    echo.
)

:vcpkg_found

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Check if executable exists
set EXE_PATH=
if exist "SlimesSkyTravel.exe" (
    set EXE_PATH=SlimesSkyTravel.exe
)
if exist "Release\SlimesSkyTravel.exe" (
    set EXE_PATH=Release\SlimesSkyTravel.exe
)
if exist "Debug\SlimesSkyTravel.exe" (
    set EXE_PATH=Debug\SlimesSkyTravel.exe
)

REM Check if CMake cache exists (indicates build environment is set up)
if exist "CMakeCache.txt" (
    REM Build environment exists - always try to build (CMake will skip if no changes)
    REM Clean CMake cache if needed (to avoid platform mismatch errors)
    REM Note: We don't clean automatically to preserve incremental builds
    
    REM Build the game
    echo Building the game...
    if defined VCPKG_TOOLCHAIN (
        call %CMAKE_EXE% .. %VCPKG_TOOLCHAIN% -G "Visual Studio 17 2022" -A x64
    ) else (
        call %CMAKE_EXE% .. -G "Visual Studio 17 2022" -A x64
    )
    if errorlevel 1 (
        echo CMake configuration failed. Please check dependencies.
        echo Make sure vcpkg is installed and dependencies are available.
        pause
        exit /b 1
    )
    
    call %CMAKE_EXE% --build . --config Release
    if errorlevel 1 (
        echo Build failed. Please check dependencies.
        pause
        exit /b 1
    )
    echo Build completed.
    
    REM Update EXE_PATH after build
    if exist "Release\SlimesSkyTravel.exe" (
        set EXE_PATH=Release\SlimesSkyTravel.exe
    ) else if exist "Debug\SlimesSkyTravel.exe" (
        set EXE_PATH=Debug\SlimesSkyTravel.exe
    ) else if exist "SlimesSkyTravel.exe" (
        set EXE_PATH=SlimesSkyTravel.exe
    )
) else (
    REM No build environment - check if executable exists
    if not defined EXE_PATH (
        echo.
        echo ========================================
        echo ERROR: No executable found and no build environment
        echo ========================================
        echo.
        echo Please build the game first or set up the build environment.
        echo.
        pause
        exit /b 1
    )
    echo Using existing executable (no build environment detected)
)

:run_game
if not defined EXE_PATH (
    echo Error: Executable not found.
    pause
    exit /b 1
)

echo Launching the game...
%EXE_PATH%
pause
