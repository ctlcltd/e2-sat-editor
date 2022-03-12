#!/bin/bash
# Pre-build and initialize an xcodeproj with qmake

cd ../src

cleanup () {
	echo "cleanup."
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm .qmake.stash
	rm qrc_resources.cpp
}

[[ "$1" == "cleanup" ]] && cleanup

echo "preparing qmake ..."
qmake -spec macx-clang e2-sat-editor.pro
echo "compiling ..."
make && qmake
echo "preparing xcodeproj ..."
qmake -spec macx-xcode
echo "done."
