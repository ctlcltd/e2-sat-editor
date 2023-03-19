#!/bin/bash
# Build with CMake targeting MinGW-64 in Linux host
# Tested under Arch Linux using mingw-w64 as guest
# 
# In order to compile, the following packages are required:
# base-devel, make, ninja, mingw-w64-cmake-static, mingw-w64-qt6-base-static, mingw-w64-curl
# 
# To compile with shared libraries, the following:
# base-devel, make, ninja, mingw-w64-cmake, mingw-w64-qt6-base, mingw-w64-curl
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Build with CMake targeting MinGW-64 in Linux host"
	fi

	printf "%s\n\n" "bash mingw.sh [OPTIONS] [-a i686] [-a x86_64]"
	printf "%s\n"   "-a --arch          Cross-compile architecture."
	printf "%s\n"   "-d --dynamic       Cross-compile dynamic libs."
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-s --strip         Task: strip"
	printf "%s\n"   "-r --release       Task: release"
	printf "%s\n"   "-e --release-early Task: release-early"
	printf "%s\n"   "-t --testing-wine  Task: testing-wine"
	printf "%s\n"   "-h --help          Display this help and exit."
}

src () {
	if [[ "$PWD" != "src"* ]]; then
		cd src

		if [[ "$PWD" != "src"* ]]; then
			echo "Directory \"src\" not found.";

			exit 1;
		fi
	fi
}

init () {
	if [[ -z $(type -t cmake) ]]; then
		echo "CMake not found."

		exit 1;
	fi

	if [[ -z "$ARCH" ]]; then
		ARCH="x86_64"
	fi

	CCMAKE="$ARCH-mingw32-cmake"
	if [[ -z "$DYNAMIC" ]]; then
		CCMAKE="$CCMAKE-static"
	fi

	if [[ -n $(type -t ninja) ]]; then
		CCNINJA="-G Ninja"
	fi
	if [[ "$ARCH" == "i686" ]]; then
		CCQT5="-DWITH_QT5=ON"
	fi

	CCSTRIP="$ARCH-mingw32-strip"
}

cleanup () {
	printf "%s\n\n" echo "cleanup."

	src
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm .qmake.stash
	rm qrc_resources.cpp
	rm -R build/.ninja_deps
	rm -R build/.ninja_log
	rm -R build/.qt
	rm -R build/.rcc
	rm -R build/CMakeFiles
	rm -R build/e2-sat-editor_autogen
	rm build/build.ninja
	rm build/CMakeCache.txt
	rm build/cmake_install.cmake
	rm build/Makefile
	rm build/*.dll
}

prepare () {
	printf "%s\n\n" "prepare."
	printf "%s\n\n" "preparing CMake ..."

	src
	$CCMAKE $CCNINJA -B build $CCQT5 -DCMAKE_BUILD_TYPE=Release
}

build () {
	printf "%s\n\n" "build."
	printf "%s\n\n" "compiling ..."

	src
	cmake --build build
}

release ()
{
	printf "%s\n\n" "release."
	printf "%s\n\n" "copying pre-built libs ..."

	src
	mkdir -p build

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
	cp ../README.md build/

	printf "%s\n\n" "NOTE: libcurl-4.dll (brotli)"
}

release_early ()
{
	printf "%s\n\n" "release-early."
	printf "%s\n\n" "copying pre-built libs ..."

	src
	mkdir -p build

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
	cp ../README.md build/
}

testing_wine ()
{
	printf "%s\n\n" "testing-wine."
	printf "%s\n\n" "copying pre-built libs ..."

	src
	mkdir -p build

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
	cp ../README.md build/
}

strip () {
	printf "%s\n\n" "strip."
	printf "%s\n\n" "binary strip ..."

	$CCSTRIP --strip-unneeded build/e2-sat-editor.exe
}

default () {
	printf "%s\n\n" "default."

	init
	prepare
	build
}

complete () {
	printf "\n%s\n" "done."
}


if [[ -z "$@" ]]; then
	usage

	exit 0
fi

for SRG in "$@"; do
	case "$SRG" in
		-a*|--arch*)
			ARCH="$2"
			init
			shift
			shift
			;;
		-d|--dynamic)
			DYNAMIC=1
			shift
			;;
		-c|--cleanup)
			cleanup
			shift
			;;
		-p|--prepare)
			init
			prepare
			shift
			;;
		-b|--build)
			init
			build
			shift
			;;
		-s|--strip)
			init
			strip
			shift
			;;
		-d|--default)
			default
			shift
			;;
		-r|--release)
			init
			prepare
			build
			release
			shift
			;;
		-e|--release-early)
			init
			prepare
			build
			release_early
			shift
			;;
		-t|--testing-wine)
			init
			prepare
			build
			testing_wine
			shift
			;;
		-h|--help)
			usage

			exit 0
			;;
		-*)
			[[ "$1" == "-"* ]] && shift
			printf "%s: %s %s\n\n" "$0" "Illegal option" "$2"

			usage 1

			exit 1
			;;
		*)
			[[ "$1" != -* ]] && usage
			;;
	esac
done

complete

