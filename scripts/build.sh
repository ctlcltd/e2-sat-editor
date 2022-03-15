#!/bin/bash
# Build in *ux with qmake and Qt 5|6

cd src

cleanup () {
	echo "cleanup."
	rm *.o
	rm moc_*.cpp
	rm moc_predefs.h
	rm Makefile
	rm .qmake.stash
	rm qrc_resources.cpp
}

if [[ -n $(type -t qmake6) ]]; then
	QMAKE="qmake6"
else [[ -n $(type -t qmake) ]]; then
	QMAKE="qmake"
else
	echo "qmake not found."
	exit 1;
fi

[[ "$1" == "cleanup" ]] && cleanup

echo "preparing qmake ..."
$QMAKE
echo "compiling ..."
make && $QMAKE
echo "done."
