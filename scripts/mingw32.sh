#!/bin/bash
# Build with CMake targeting mingw32-w64 in linux host
# Tested under Arch Linux with mingw-w64 shared libs
# In order to compile: mingw-w64-cmake, mingw-w64-qt6, mingw-w64-curl, are required

cd src

cleanup () {
	echo "cleanup."
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm .qmake.stash
	rm qrc_resources.cpp
	rm -R build/.rcc
	rm -R build/CMakeFiles
	rm -R build/e2-sat-editor_autogen
	rm build/CMakeCache.txt
	rm build/cmake_install.cmake
	rm build/Makefile
}

if [[ -z $(type -t cmake) ]]; then
	echo "cmake not found."
	exit 1;
fi

echo "preparing cmake ..."
x86_64-w64-mingw32-cmake -B build
echo "compiling ..."
cmake --build build

echo "copying pre-built libs ..."
cp -R /usr/x86_64-w64-mingw32/lib/qt6/plugins/platforms build/
cp -R /usr/x86_64-w64-mingw32/lib/qt6/plugins/styles build/
cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll build/
cp /usr/x86_64-w64-mingw32/bin/libstdc++-6.dll build/
cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Core.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Gui.dll build/
cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Widgets.dll build/

echo "done."

# x86_64-w64-mingw32-wine build/e2-sat-editor.exe
