#!/bin/bash
# Pre-build and initialize an xcodeproj with qmake

rm *.o
rm moc_*.cpp
rm moc_predefs.h
rm Makefile
rm .qmake.stash
rm qrc_resources.cpp
qmake -spec macx-clang e2-sat-editor.pro
make && qmake
#qmake -spec macx-xcode
