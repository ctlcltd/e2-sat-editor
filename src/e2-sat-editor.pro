CONFIG += qt debug
QT += widgets
SOURCES += main.cpp
SOURCES += e2db.cpp
SOURCES += ftpcom.cpp
SOURCES += gui/*.cpp
HEADERS += gui/*.h
INCLUDEPATH += ../vendor/curl/include
LIBS += -L../vendor/curl/lib -lcurl
RESOURCES = ../res/resources.qrc
QMAKE_CXXFLAGS += -std=c++17
macx {
	TARGET = e2" "SAT" "Editor
} else {
	TARGET = e2-sat-editor
}
ICON = ../dist/macos/e2-sat-editor.icns
RC_ICONS = ../dist/windows/e2-sat-editor.ico
