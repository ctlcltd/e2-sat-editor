#!/bin/bash
# Build with CMake targeting mingw32-w64 in linux host
# Tested under Arch Linux using mingw-w64 as guest
# 
# In order to compile, the following packages are required:
# base-devel, make, ninja, mingw-w64-cmake-static, mingw-w64-qt6-base-static, mingw-w64-curl
# 
# To compile with shared libraries, the following:
# base-devel, make, ninja, mingw-w64-cmake, mingw-w64-qt6-base, mingw-w64-curl
# 

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

prebuilt_release ()
{
	echo "copying pre-built libs ..."
	# x86_64 qt6
	cp /usr/x86_64-w64-mingw32/bin/libbrotlicommon.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libbrotlidec.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libbz2-1.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libfreetype-6.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libglib-2.0-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libgraphite2.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libharfbuzz-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libiconv-2.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libintl-8.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libjpeg-8.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libpcre2-8-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libpcre2-16-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libpng16-16.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libssp-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libstdc++-6.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll build/
	cp /usr/x86_64-w64-mingw32/bin/zlib1.dll build/
	# libcurl
	cp /usr/x86_64-w64-mingw32/bin/libcrypto-3-x64.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libcurl-4.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libidn2-0.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libnghttp2-14.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libpsl-5.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libssh2.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libssl-3-x64.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libunistring-5.dll build/
	# e2se
	cp {COPYING,LICENSE} build/
}

prebuilt_release_early ()
{
	echo "copying pre-built libs ..."
	# i686 qt5
	cp /usr/i686-w64-mingw32/bin/libbrotlicommon.dll build/
	cp /usr/i686-w64-mingw32/bin/libbrotlidec.dll build/
	cp /usr/i686-w64-mingw32/bin/libbz2-1.dll build/
	cp /usr/i686-w64-mingw32/bin/libfreetype-6.dll build/
	cp /usr/i686-w64-mingw32/bin/libgcc_s_dw2-1.dll build/
	cp /usr/i686-w64-mingw32/bin/libglib-2.0-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libgraphite2.dll build/
	cp /usr/i686-w64-mingw32/bin/libharfbuzz-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libiconv-2.dll build/
	cp /usr/i686-w64-mingw32/bin/libintl-8.dll build/
	cp /usr/i686-w64-mingw32/bin/libjpeg-8.dll build/
	cp /usr/i686-w64-mingw32/bin/libpcre2-8-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libpcre2-16-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libpng16-16.dll build/
	cp /usr/i686-w64-mingw32/bin/libssp-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libstdc++-6.dll build/
	cp /usr/i686-w64-mingw32/bin/libwinpthread-1.dll build/
	cp /usr/i686-w64-mingw32/bin/zlib1.dll build/
	# qt5-base
	cp /usr/i686-w64-mingw32/bin/Qt5Core.dll build/
	cp /usr/i686-w64-mingw32/bin/Qt5Gui.dll build/
	cp /usr/i686-w64-mingw32/bin/Qt5Widgets.dll build/
	cp /usr/i686-w64-mingw32/bin/Qt5PrintSupport.dll build/
	mkdir -p build/{platforms,styles}
	cp /usr/i686-w64-mingw32/lib/qt/plugins/platforms/qwindows.dll build/platforms
	cp /usr/i686-w64-mingw32/lib/qt/plugins/styles/qwindowsvistastyle.dll build/styles
	# libcurl
	cp /usr/i686-w64-mingw32/bin/libcrypto-3.dll build/
	cp /usr/i686-w64-mingw32/bin/libcurl-4.dll build/
	cp /usr/i686-w64-mingw32/bin/libidn2-0.dll build/
	cp /usr/i686-w64-mingw32/bin/libnghttp2-14.dll build/
	cp /usr/i686-w64-mingw32/bin/libpsl-5.dll build/
	cp /usr/i686-w64-mingw32/bin/libssh2.dll build/
	cp /usr/i686-w64-mingw32/bin/libssl-3.dll build/
	cp /usr/i686-w64-mingw32/bin/libunistring-5.dll build/
	# e2se
	cp {COPYING,LICENSE} build/
}

prebuilt_mingw32_wine ()
{
	echo "copying pre-built libs ..."
	# x86_64 qt6
	cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libstdc++-6.dll build/
	cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll build/
	# qt6-base
	cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Core.dll build/
	cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Gui.dll build/
	cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6Widgets.dll build/
	cp /usr/x86_64-w64-mingw32/lib/qt6/bin/Qt6PrintSupport.dll build/
	mkdir -p build/{platforms,styles}
	cp /usr/x86_64-w64-mingw32/lib/qt6/plugins/platforms/qwindows.dll build/platforms
	cp /usr/x86_64-w64-mingw32/lib/qt6/plugins/styles/qwindowsvistastyle.dll build/styles
	# e2se
	cp {COPYING,LICENSE} build/
}


if [[ -z $(type -t cmake) ]]; then
	echo "cmake not found."
	exit 1;
fi


[[ "$1" == "cleanup" ]] && cleanup

echo "preparing cmake ..."

# x86_64-w64-mingw32-cmake-static -G Ninja -B build -DCMAKE_BUILD_TYPE=Release
i686-w64-mingw32-cmake-static -G Ninja -B build -DWITH_QT5=ON -DCMAKE_BUILD_TYPE=Release

# x86_64-w64-mingw32-cmake -G Ninja -B build
# x86_64-w64-mingw32-cmake -B build

echo "compiling ..."
cmake --build build

[[ "$1" == "release" ]] && prebuilt_release
[[ "$1" == "release-early" ]] && prebuilt_release_early
[[ "$1" == "testing" ]] && prebuilt_mingw32_wine

echo "done."

# x86_64-w64-mingw32-strip --strip-unneeded build/e2-sat-editor.exe
# x86_64-w64-mingw32-wine build/e2-sat-editor.exe

