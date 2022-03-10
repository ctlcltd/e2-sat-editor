#!/bin/bash
# Build in *ux with qmake and Qt 5|6

if [[ -n $(type -t qmake6) ]]; then
	QMAKE="qmake6"
else
	QMAKE="qmake"
fi

#rm *.o
#rm moc_*.cpp
#rm moc_predefs.h
#rm Makefile
#rm .qmake.stash
#rm qrc_resources.cpp
$QMAKE
make && $QMAKE
