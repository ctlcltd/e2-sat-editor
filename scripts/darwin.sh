#!/bin/bash
# Build or initialize an xcodeproj with QMake
# 

usage () {
	if [[ -z "$1" ]]; then
		printf "%s\n\n" "Build or initialize an xcodeproj with QMake"
	fi

	printf "%s\n\n" "bash darwin.sh [OPTIONS] [-b debug | release]"
	printf "%s\n"   "-q --qmake         QMake executable."
	printf "%s\n"   "-c --cleanup       Task: cleanup"
	printf "%s\n"   "-p --prepare       Task: prepare"
	printf "%s\n"   "-b --build         Task: build"
	printf "%s\n"   "-x --xcodeproj     Task: xcodeproj"
	printf "%s\n"   "-d --default       Task: default"
	printf "%s\n"   "-r --release       Task: release"
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

fixes () {
	cd Frameworks

	# missing: libbrotlicommon.1.dylib in: Frameworks/libbrotlidec.1.dylib
	cp /usr/local/Cellar/brotli/1.1.0/lib/libbrotlicommon.1.1.0.dylib libbrotlicommon.1.dylib
	install_name_tool -id @rpath/libbrotlicommon.1.dylib "libbrotlicommon.1.dylib"
	codesign --force -s - "libbrotlicommon.1.dylib"
	# wrong path: libbrotlicommon.1.dylib in: Frameworks/libbrotlidec.1.dylib
	install_name_tool -change /usr/local/Cellar/brotli/1.1.0/lib/libbrotlicommon.1.dylib @rpath/libbrotlicommon.1.dylib "libbrotlidec.1.dylib"

	# wrong path: libldap.2.dylib in: Frameworks/libgthread-2.0.0.dylib
	install_name_tool -change /usr/local/Cellar/glib/2.78.3/lib/libglib-2.0.0.dylib @rpath/libglib-2.0.0.dylib "libgthread-2.0.0.dylib"

	# wrong path: libicuuc.73.dylib in: Frameworks/libicui18n.73.dylib
	install_name_tool -change @loader_path/libicuuc.73.dylib @rpath/libicuuc.73.dylib "libicui18n.73.dylib"
	# wrong path: libicudata.73.dylib in Frameworks/libicui18n.73.dylib
	install_name_tool -change @loader_path/libicudata.73.dylib @rpath/libicudata.73.dylib "libicui18n.73.dylib"
	# wrong path: libicudata.73.dylib in: Frameworks/libicuuc.73.dylib
	install_name_tool -change @loader_path/libicudata.73.dylib @rpath/libicudata.73.dylib "libicuuc.73.dylib"

	# wrong path: liblber.2.dylib in: Frameworks/libldap.2.dylib
	install_name_tool -change @loader_path/liblber.2.dylib @rpath/liblber.2.dylib "libldap.2.dylib"
	# wrong path: libssl.3.dylib in: Frameworks/libssl.3.dylib
	install_name_tool -change @loader_path/../../../../opt/openssl@3/lib/libssl.3.dylib @rpath/libssl.3.dylib "libldap.2.dylib"
	# wrong path: libcrypto.3.dylib in: Frameworks/libcrypto.3.dylib
	install_name_tool -change @loader_path/../../../../opt/openssl@3/lib/libcrypto.3.dylib @rpath/libcrypto.3.dylib "libldap.2.dylib"

	# wrong path: /../lib/ in: Frameworks/libmd4c.0.dylib
	install_name_tool -delete_rpath @loader_path/../lib "libmd4c.0.dylib"

	# wrong path: libcrypto.3.dylib in: Frameworks/libssl.3.dylib
	install_name_tool -change /usr/local/Cellar/openssl@3/3.2.0_1/lib/libcrypto.3.dylib @rpath/libcrypto.3.dylib "libssl.3.dylib"

	# wrong path: /../lib in: Frameworks/libzstd.1.dylib
	install_name_tool -delete_rpath @loader_path/../lib "libzstd.1.dylib"

	# wrong path: /../../../ in: Frameworks/*.framework
	install_name_tool -delete_rpath @loader_path/../../../ "QtCore.framework/Versions/A/QtCore"
	install_name_tool -delete_rpath @loader_path/../../../ "QtDBus.framework/Versions/A/QtDBus"
	install_name_tool -delete_rpath @loader_path/../../../ "QtGui.framework/Versions/A/QtGui"
	install_name_tool -delete_rpath @loader_path/../../../ "QtPrintSupport.framework/Versions/A/QtPrintSupport"
	install_name_tool -delete_rpath @loader_path/../../../ "QtWidgets.framework/Versions/A/QtWidgets"

	install_name_tool -add_rpath @executable_path/../Frameworks "QtCore.framework/Versions/A/QtCore"
	install_name_tool -add_rpath @executable_path/../Frameworks "QtDBus.framework/Versions/A/QtDBus"
	install_name_tool -add_rpath @executable_path/../Frameworks "QtGui.framework/Versions/A/QtGui"
	install_name_tool -add_rpath @executable_path/../Frameworks "QtPrintSupport.framework/Versions/A/QtPrintSupport"
	install_name_tool -add_rpath @executable_path/../Frameworks "QtWidgets.framework/Versions/A/QtWidgets"

	install_name_tool -add_rpath @loader_path/Frameworks "QtCore.framework/Versions/A/QtCore"
	install_name_tool -add_rpath @loader_path/Frameworks "QtDBus.framework/Versions/A/QtDBus"
	install_name_tool -add_rpath @loader_path/Frameworks "QtGui.framework/Versions/A/QtGui"
	install_name_tool -add_rpath @loader_path/Frameworks "QtPrintSupport.framework/Versions/A/QtPrintSupport"
	install_name_tool -add_rpath @loader_path/Frameworks "QtWidgets.framework/Versions/A/QtWidgets"

	cd ../PlugIns

	# wrong path: /../../../../lib in: PlugIns/*
	install_name_tool -delete_rpath @loader_path/../../../../lib "platforms/libqcocoa.dylib"
	install_name_tool -delete_rpath @loader_path/../../../../lib "styles/libqmacstyle.dylib"

	install_name_tool -add_rpath @executable_path/../Frameworks "platforms/libqcocoa.dylib"
	install_name_tool -add_rpath @executable_path/../Frameworks "styles/libqmacstyle.dylib"

	codesign --force -s - "platforms/libqcocoa.dylib"
	codesign --force -s - "styles/libqmacstyle.dylib"

	cd ..

	# wrong path: /usr/local/lib in: e2 SAT Editor
	install_name_tool -delete_rpath /usr/local/lib "e2 SAT Editor"
	install_name_tool -delete_rpath @executable_path/../Frameworks "e2 SAT Editor"

	install_name_tool -add_rpath @loader_path/ "e2 SAT Editor"
	install_name_tool -add_rpath @executable_path/../MacOS "e2 SAT Editor"
	install_name_tool -add_rpath @executable_path/../Frameworks "e2 SAT Editor"
}

release () {
	printf "%s\n\n" "release."

	src

	mkdir -p build/Frameworks
	mkdir -p build/PlugIns
	mkdir -p build/translations

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

	mkdir PlugIns/{platforms,styles}
	cp /usr/local/share/qt/plugins/platforms/libqcocoa.dylib PlugIns/platforms/libqcocoa.dylib
	cp /usr/local/share/qt/plugins/styles/libqmacstyle.dylib PlugIns/styles/libqmacstyle.dylib

	printf "%s\n\n" "applying current fixes libs ..."

	fixes

	cd ..

	mkdir -p "e2 SAT Editor.app/Contents/Frameworks"
	mkdir -p "e2 SAT Editor.app/Contents/PlugIns"
	cp -R build/{Frameworks,PlugIns} "e2 SAT Editor.app/Contents/"

	printf "%s\n\n" "copying translations ..."

	mkdir -p "e2 SAT Editor.app/Contents/Resources/translations"
	cp ../dist/translations/*.qm build/translations
	cp /usr/local/share/qt/translations/qt_*.qm build/translations
	cp /usr/local/share/qt/translations/qtbase_*.qm build/translations
	rm -R build/translations/qt_help_*.qm
	cp -R build/translations "e2 SAT Editor.app/Contents/Resources/"

	printf "%s\n\n" "copying executable ..."
	cp "build/e2 SAT Editor" "e2 SAT Editor.app/Contents/MacOS/"

	printf "%s\n\n" "fixing Info.plist ..."
	cp "../dist/macos/Info.plist.in" "e2 SAT Editor.app/Contents/Info.plist"
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

