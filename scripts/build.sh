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

if [[ -z $QMAKE ]]; then
	if [[ -n $(type -t qmake6) ]]; then
		QMAKE="qmake6"
	elif [[ -n $(type -t qmake) ]]; then
		QMAKE="qmake"
	fi
fi
if [[ -z $(type -t $QMAKE) ]]; then
	echo "qmake not found."
	exit 1;
fi
if [[ -z $(type -t make) ]]; then
	echo "make not found."
	exit 1;
fi

[[ "$1" == "cleanup" ]] && cleanup

echo "preparing qmake ..."
$QMAKE
echo "compiling ..."
make && $QMAKE
echo "done."
