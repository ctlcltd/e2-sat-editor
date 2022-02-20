#!/bin/bash
# Build in *ux with qmake and Qt 5|6

if [[ -n $(type -t qmake6) ]]; then
	QMAKE = "qmake6"
else
	QMAKE = "qmake"
fi

$QMAKE
make && $QMAKE
