#!/bin/bash
qmake -spec macx-clang e2-sat-editor.pro
make && qmake
qmake -spec macx-xcode
