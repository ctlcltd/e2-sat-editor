#!/bin/bash
# Build with CMake targeting mingw32-w64 in linux host
# Tested under Arch Linux with mingw-w64 shared libs
# In order to compile: mingw-w64-cmake, mingw-w64-qt6, mingw-w64-curl, are required

cd src
x86_64-w64-mingw32-cmake -B build
cmake --build build

cp -R /usr/x86_64-w64-mingw32/lib/qt6/plugins/platforms build/
cp -R /usr/x86_64-w64-mingw32/lib/qt6/plugins/styles build/
cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll build/
cp /usr/x86_64-w64-mingw32/bin/libstdc++-6.dll build/
cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Core.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Gui.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Widgets.dll build/

# x86_64-w64-mingw32-wine build/e2-sat-editor.exe
