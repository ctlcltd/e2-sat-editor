#!/bin/bash
# Build with CMake targeting mingw32 in linux host
# Tested under Arch Linux with mingw-w64 shared libs

cd src
x86_64-w64-mingw32-cmake -B build
cmake --build build
