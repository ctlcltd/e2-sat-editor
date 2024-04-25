#!/bin/bash
# Build cross-compile with CMake targeting MinGW-64 in Linux host
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
		printf "%s\n\n" "Build cross-compile with CMake targeting MinGW-64 in Linux host"
	fi

	printf "%s\n\n" "bash build-mingw.sh [OPTIONS] [-a i686] [-a x86_64] [-p debug | release]"
	printf "%s\n"   "-a --arch          Cross-compile architecture."
	printf "%s\n"   "-d --dynamic       Cross-compile dynamic libs."
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-s --strip         Task: strip"
	printf "%s\n"   "-r --release       Task: release"
	printf "%s\n"   "-e --release-early Task: release-early"
	printf "%s\n"   "-h --help          Display this help and exit."
}

#TODO improve
src () {
	if [[ $(basename $PWD) != "src" ]]; then
		cd src

		if [[ $(basename $PWD) != "src" ]]; then
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

	CCMAKE="$ARCH-w64-mingw32-cmake"
	if [[ -z "$DYNAMIC" ]]; then
		CCMAKE="$CCMAKE-static"
	fi

	if [[ -n $(type -t ninja) ]]; then
		CCNINJA="-G Ninja"
	fi
	if [[ "$ARCH" == "i686" ]]; then
		CCQT5="-DWITH_QT5=ON"
	fi

	CCSTRIP="$ARCH-w64-mingw32-strip"

	if [[ "$TARGET" == "debug" ]]; then
		CCTARGET="Debug"
	else
		CCTARGET="Release"
	fi
}

compiler () {
	local compiler="CMake MinGW-64"

	if [[ -n "$ARCH" ]]; then
		compiler="$compiler $ARCH"
	fi
	if [[ -n "$CCNINJA" ]]; then
		compiler="$compiler Ninja"
	fi
	if [[ -n "$CCQT5" ]]; then
		compiler="$compiler Qt5"
	else
		compiler="$compiler Qt6"
	fi

	printf "%s\n\n" "$compiler"
}

cleanup () {
	printf "%s\n\n" "cleanup."

	src
	rm -R *.o
	rm -R moc_*.cpp
	rm -R moc_predefs.h
	rm Makefile
	rm Makefile.Debug
	rm Makefile.Release
	rm .qmake.stash
	rm qrc_resources*.cpp
	rm -R build
}

prepare () {
	printf "%s\n\n" "prepare."

	compiler

	printf "%s\n\n" "preparing CMake ..."

	src
	mkdir -p build
	$CCMAKE $CCNINJA -B build $CCQT5 -DCMAKE_BUILD_TYPE=$CCTARGET
}

build () {
	printf "%s\n\n" "build."

	compiler

	printf "%s\n\n" "compiling ..."

	src
	mkdir -p build
	cmake --build build
}

release () {
	printf "%s\n\n" "release."

	src

	printf "%s\n\n" "pre actions ..."

	#TODOFIX qt6-translations from mingw package
	mkdir -p build/translations
	cp ../dist/translations/*.qm build/translations
	cp /usr/share/qt6/translations/qt_*.qm build/translations
	cp /usr/share/qt6/translations/qtbase_*.qm build/translations
	rm -R build/translations/qt_help_*.qm

	printf "%s\n\n" "run deploy script ..."

	bash ../scripts/deployqtwin.sh "build/e2-sat-editor.exe" \
		--verbose \
		-modules Core,Gui,Widgets,PrintSupport \
		-libraries libcurl-4.dll,libcrypto-3-x64.dll,libjpeg-8.dll,libidn2-0.dll,libnghttp2-14.dll,libpsl-5.dll,libssh2.dll,libssl-3-x64.dll,libunistring-5.dll

	printf "%s\n\n" "post actions ..."

	rm "build/platforms/qdirect2d.dll"

	local tip="Use official win x86_64 release from curl. Rename libcurl-x64.dll to libcurl-4.dll"
	printf "%s\n%s\n\n" "Note: libcurl-4.dll (brotli) could be broken." "$tip"
}

release_early () {
	printf "%s\n\n" "release-early."

	src

	printf "%s\n\n" "pre actions ..."

	#TODOFIX qt6-translations from mingw package
	mkdir -p build/translations
	cp ../dist/translations/*.qm build/translations
	cp /usr/share/qt6/translations/qt_*.qm build/translations
	cp /usr/share/qt6/translations/qtbase_*.qm build/translations
	rm -R build/translations/qt_help_*.qm

	printf "%s\n\n" "run deploy script ..."

	bash ../scripts/deployqtwin.sh "build/e2-sat-editor.exe" \
		--verbose \
		-system MINGW32 \
		-qt-version 5 \
		-modules Core,Gui,Widgets,PrintSupport \
		-plugins platforms,styles,printsupport \
		-libraries libcurl-4.dll,libcrypto-3.dll,libjpeg-8.dll,libidn2-0.dll,libnghttp2-14.dll,libpsl-5.dll,libssh2.dll,libssl-3.dll,libunistring-5.dll

	printf "%s\n\n" "post actions ..."

	rm "build/platforms/qdirect2d.dll"
}

strip () {
	printf "%s\n\n" "strip."

	src

	printf "%s\n\n" "binary strip ..."

	$CCSTRIP --strip-unneeded build/e2-sat-editor.exe
	$CCSTRIP --strip-unneeded build/cli/e2se-cli.exe
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
		-p*|--prepare*)
			TARGET="$2"
			init
			prepare
			shift
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
			strip
			release
			shift
			;;
		-e|--release-early)
			ARCH="i686"
			init
			prepare
			build
			strip
			release_early
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

