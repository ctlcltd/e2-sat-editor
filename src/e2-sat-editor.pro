CONFIG += qt debug
QT += widgets
QT += printsupport
SOURCES += main.cpp
SOURCES += logger/*.cpp
SOURCES += ftpcom/*.cpp
SOURCES += e2db/*.cpp
SOURCES += gui/toolkit/*.cpp
SOURCES += gui/*.cpp
HEADERS += logger/*.h
HEADERS += ftpcom/*.h
HEADERS += e2db/*.h
HEADERS += gui/platforms/platform.h
HEADERS += gui/toolkit/*.h
HEADERS += gui/*.h
INCLUDEPATH += ../vendor/curl/include
LIBS += -L../vendor/curl/lib -lcurl
RESOURCES = ../res/resources.qrc
QMAKE_CXXFLAGS += -std=c++17
macx {
	HEADERS += gui/platforms/platform_macx.h
	SOURCES += gui/platforms/platform_macx.mm
	TARGET = e2" "SAT" "Editor
} else {
	TARGET = e2-sat-editor
}
ICON = ../dist/macos/e2-sat-editor.icns
RC_ICONS = ../dist/windows/e2-sat-editor.ico
