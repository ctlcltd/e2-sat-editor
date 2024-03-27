#!/bin/bash
# Build or initialize an xcodeproj with QMake
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Build or initialize an xcodeproj with QMake"
	fi

	printf "%s\n\n" "bash build-darwin.sh [OPTIONS] [-b debug | release]"
	printf "%s\n"   "-q --qmake         QMake executable."
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-x --xcodeproj     Task: xcodeproj"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-r --release       Task: release"
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
	if [[ -z "$QMAKE" ]]; then
		if [[ -n $(type -t qmake) ]]; then
			QMAKE="qmake"
		fi
	fi

	if [[ -z $(type -t qmake) ]]; then
		echo "QMake not found."

		exit 1;
	fi
	if [[ -z $(type -t make) ]]; then
		echo "Make not found."

		exit 1;
	fi

	if [[ "$TARGET" == "debug" ]]; then
		TARGET="debug"
	else
		TARGET="release"
	fi
}

compiler () {
	local compiler="QMake"

	if [[ -n "$TARGET" ]]; then
		compiler="$compiler $TARGET"
	fi

	printf "%s\n\n" "$compiler"

	$QMAKE --version
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
	rm -R debug
	rm -R release
}

prepare () {
	printf "%s\n\n" "prepare."
	compiler
	printf "%s\n\n" "preparing QMake ..."

	src
	$QMAKE -spec macx-clang e2-sat-editor.pro
}

build () {
	printf "%s\n\n" "build."
	compiler
	printf "%s\n\n" "compiling ..."

	src
	make $TARGET && $QMAKE
}

xcodeproj () {
	printf "%s\n\n" "xcodeproj."
	compiler
	printf "%s\n\n" "preparing xcodeproj ..."

	src
	$QMAKE -spec macx-xcode
}

release () {
	printf "%s\n\n" "release."

	src

	cp -R "e2 SAT Editor.app" "build/e2 SAT Editor.app"

	printf "%s\n\n" "pre actions ..."

	QMDIR="build/e2 SAT Editor.app/Contents/Resources/translations"

	mkdir -p "$QMDIR"
	cp ../dist/translations/*.qm "$QMDIR"
	cp /usr/local/share/qt/translations/qt_*.qm "$QMDIR"
	cp /usr/local/share/qt/translations/qtbase_*.qm "$QMDIR"
	rm -R "$QMDIR"/qt_help_*.qm

	cp "../dist/macos/Info.plist.in" "build/e2 SAT Editor.app/Contents/Info.plist"
	cp "../dist/macos/PkgInfo.in" "build/e2 SAT Editor.app/Contents/PkgInfo"

	printf "%s\n\n" "run deploy script ..."

	bash ../scripts/deployqtmacx.sh "build/e2 SAT Editor.app" --verbose

	chmod +x "build/e2 SAT Editor.app/Contents/MacOS/e2 SAT Editor"

	printf "%s\n\n" "copying e2se files ..."

	cp ../dist/common/Readme.txt.in build/Readme.txt
	cp COPYING build/License.txt
}

default () {
	printf "%s\n\n" "default."

	init
	prepare
	build
	xcodeproj
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
		-q*|--qmake*)
			QMAKE="$2"
			init
			shift
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
		-b*|--build*)
			TARGET="$2"
			init
			build
			shift
			shift
			;;
		-x|--xcodeproj)
			init
			prepare
			xcodeproj
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

