CONFIG += qt debug
QT += widgets
SOURCES += main.cpp
SOURCES += e2db.cpp
SOURCES += gui/*.cpp
HEADERS += gui/*.h
RESOURCES = ../res/resources.qrc
QMAKE_CXXFLAGS += -std=c++17
