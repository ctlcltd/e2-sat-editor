#!/bin/bash
# Build or initialize an xcodeproj with QMake
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Build or initialize an xcodeproj with QMake"
	fi

	printf "%s\n\n" "bash darwin.sh [OPTIONS]"
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-x --xcodeproj     Task: xcodeproj"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-r --release       Task: release"
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
	if [[ -z $(type -t qmake) ]]; then
		echo "QMake not found."

		exit 1;
	fi
	if [[ -z $(type -t make) ]]; then
		echo "Make not found."

		exit 1;
	fi
}

cleanup () {
	printf "%s\n\n" "cleanup."

	src
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm Makefile.Debug
	rm Makefile.Release
	rm .qmake.stash
	rm qrc_resources.cpp
}

prepare () {
	printf "%s\n\n" "prepare."
	printf "%s\n\n" "preparing QMake ..."

	src
	qmake -spec macx-clang e2-sat-editor.pro
}

build () {
	printf "%s\n\n" "build."
	printf "%s\n\n" "compiling ..."

	src
	make && qmake
}

xcodeproj () {
	printf "%s\n\n" "xcodeproj."
	printf "%s\n\n" "preparing xcodeproj ..."

	src
	qmake -spec macx-xcode
}

cpframework () {
	local src="$1"
	local dst="$2"
	local files=$(ls "$src")

	for filename in $files; do
		if [[ "$filename" != "Headers" ]]; then
			if [[ -d "$src/$filename" ]]; then
				mkdir -p "$dst/$filename"
				cpframework "$src/$filename" "$dst/$filename"
			elif [[ -f "$src/$filename" ]]; then
				cp -R  "$src/$filename" "$dst/$filename"
			fi
		fi
	done
}

relinking () {
	local qtbase="/usr/local/opt/qt/lib"

	if [[ -n "$1" ]]; then
		local libs=$(dyld_info -dependents "$1" | tail -n +2)

		for lib in $libs; do
			if [[ "$lib" == "/usr/local/opt"* || "$lib" == "@rpath"* ]]; then
				# echo -e "dylib: $lib"

				local srcname="${lib#@rpath\/}"
				if [[ "$lib" == "@rpath"* && "$srcname" == *".framework"* ]]; then
					srcname="$qtbase/$srcname"
				fi
				local dstname="${srcname##*/}"
				if [[ "$srcname" == *".framework"* ]]; then
					dstname="${srcname##$qtbase\/}"
				fi
				dstname="Frameworks/$dstname"

				# echo -e "dstname: $dstname"

				if [[ "$srcname" == *".framework"* ]]; then
					local src="${srcname##$qtbase\/}"
					local name="${src##*/}"
					src="$qtbase/$name.framework"
					local dst="Frameworks/$name.framework"

					cpframework $src $dst
				else
					cp "$srcname" "$dstname"
				fi

				local name=$(otool -D "$dstname" | tail -n +2)

				if [[ "$name" == "/usr/local/opt"* ]]; then
					local rpath="$dstname"
					if [[ "$rpath" == *".framework"* ]]; then
						rpath="${rpath##$qtbase\/}"
						rpath="${rpath##Frameworks\/}"
					else
						rpath="${rpath##*/}"
					fi
					rpath="@rpath/$rpath"

					# echo -e "name: $rpath"

					install_name_tool -id $rpath "$dstname"
					codesign --force -s - "$dstname"
				fi

				local rpath="$dstname"
				if [[ "$rpath" == *".framework"* ]]; then
					rpath="${rpath##$qtbase\/}"
					rpath="${rpath##Frameworks\/}"
				else
					rpath="${rpath##*/}"
				fi
				rpath="@rpath/$rpath"

				# echo -e "rpath: $rpath"

				install_name_tool -change $lib $rpath "$1"
				codesign --force -s - "$dstname"

				relinking "$dstname"
			fi
		done
	fi
}

release () {
	printf "%s\n\n" "release."

	src

	mkdir -p build/Frameworks

	cp -R "e2 SAT Editor.app" "e2 SAT Editor.app.bak"
	cp "e2 SAT Editor.app/Contents/MacOS/e2 SAT Editor" "build/e2 SAT Editor"

	printf "%s\n\n" "copying pre-built libs ..."

	cd build

	relinking "e2 SAT Editor"

	for name in "QtCore" "QtDBus" "QtGui" "QtPrintSupport" "QtWidgets"; do
		rm -Rf "Frameworks/$name.framework/Resources"
		rm -Rf "Frameworks/$name.framework/Versions/Current"
		rm -Rf "Frameworks/$name.framework/Versions/A/_CodeSignature"
		cd "Frameworks/$name.framework/Versions"
		ln -s A Current
		cd ../../..
		cd "Frameworks/$name.framework"
		ln -s Versions/A/Resources Resources
		cd ../..
	done

	cd ..
	mkdir -p "e2 SAT Editor.app/Contents/Frameworks"
	cp -R build/Frameworks "e2 SAT Editor.app/Contents/"
	cp "build/e2 SAT Editor" "e2 SAT Editor.app/Contents/MacOS/"
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
		-b|--build)
			init
			build
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

