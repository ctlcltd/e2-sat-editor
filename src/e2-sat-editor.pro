CONFIG += qt debug
QT += widgets
SOURCES += main.cpp
SOURCES += e2db.cpp
SOURCES += gui/*.cpp
HEADERS += gui/*.h
INCLUDEPATH += ../vendor/curl/include
LIBS += -L../vendor/curl/lib -lcurl
RESOURCES = ../res/resources.qrc
QMAKE_CXXFLAGS += -std=c++17
