#!/bin/bash
# Pre-build and initialize an xcodeproj with qmake

qmake -spec macx-clang e2-sat-editor.pro
make && qmake
qmake -spec macx-xcode
