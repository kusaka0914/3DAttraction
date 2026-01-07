#!/bin/bash

echo "Starting the game..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

cd build

# Determine executable name (no extension on macOS, .exe on Windows)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    EXECUTABLE="SlimesSkyTravel.exe"
else
    EXECUTABLE="SlimesSkyTravel"
fi

# Check if build is needed
if [ ! -f "$EXECUTABLE" ] || [ "../CMakeLists.txt" -nt "$EXECUTABLE" ]; then
    echo "Building the game..."
    cmake .. && make
    if [ $? -ne 0 ]; then
        echo "Build failed. Please check dependencies."
        exit 1
    fi
    echo "Build completed."
else
    echo "Using existing build."
fi

echo "Launching the game..."
./"$EXECUTABLE"