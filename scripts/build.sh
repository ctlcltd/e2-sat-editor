#!/bin/bash
# Build in *ux with QMake
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Build in *ux with QMake"
	fi

	printf "%s\n\n" "bash build.sh [OPTIONS] [-q qmake] [-q qmake6] [-b debug | release]"
	printf "%s\n"   "-q --qmake         QMake executable."
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-h --help          Display this help and exit."
}

#//TODO improve
src () {
	if [[ $(basename $PWD) != "src" ]]; then
		cd src

		if [[ $(basename $PWD) != "src" ]]; then
			echo "Directory \"src\" not found.";

			exit 1;
		fi
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

init () {
	if [[ -z "$QMAKE" ]]; then
		if [[ -n $(type -t qmake6) ]]; then
			QMAKE="qmake6"
		elif [[ -n $(type -t qmake) ]]; then
			QMAKE="qmake"
		fi
	fi

	if [[ -z $(type -t "$QMAKE") ]]; then
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
	$QMAKE
}

build () {
	printf "%s\n\n" "build."
	compiler
	printf "%s\n\n" "compiling ..."

	src
	make $TARGET && $QMAKE
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
		-d|--default)
			default
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

