#!/bin/bash
# Pre-build and initialize an xcodeproj with qmake
# 

cd src

cleanup () {
	echo "cleanup."
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm Makefile.Debug
	rm Makefile.Release
	rm .qmake.stash
	rm qrc_resources.cpp
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
		local libs=$(xcrun dyldinfo -dylibs "$1" | tail -n +2)

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

prebuilt_release () {
	echo "release."

	make release

	mkdir -p build/Frameworks

	cp -R "e2 SAT Editor.app" "e2 SAT Editor.app.bak"
	cp "e2 SAT Editor.app/Contents/MacOS/e2 SAT Editor" "build/e2 SAT Editor"

	echo "copying pre-built libs ..."

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


if [[ -z $(type -t qmake) ]]; then
	echo "qmake not found."
	exit 1;
fi
if [[ -z $(type -t make) ]]; then
	echo "make not found."
	exit 1;
fi

[[ "$1" == "cleanup" ]] && cleanup


echo "preparing qmake ..."
qmake -spec macx-clang e2-sat-editor.pro

echo "compiling ..."
make && qmake

echo "preparing xcodeproj ..."
qmake -spec macx-xcode

[[ "$1" == "release" ]] && prebuilt_release

echo "done."

