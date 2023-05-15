VERSION = 0.6.0
QMAKE_TARGET_BUNDLE_PREFIX = org.e2se
QMAKE_BUNDLE = e2-sat-editor
QMAKE_APPLICATION_BUNDLE_NAME = e2" "SAT" "Editor

CONFIG += qt debug_and_release c++17
QT += widgets
QT += printsupport

SOURCES += main.cpp
SOURCES += logger/*.cpp
SOURCES += ftpcom/*.cpp
SOURCES += e2db/*.cpp
SOURCES += gui/toolkit/*.cpp
SOURCES += gui/*.cpp

HEADERS += e2se_defs.h
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

unix:!mac {
	isEmpty (PREFIX):PREFIX = /usr

	target.path = $$PREFIX/bin
	target.files += $$TARGET
	desktop.path = $$PREFIX/share/applications
	desktop.files += ../dist/xdg/e2-sat-editor.desktop
	icon.path = $$PREFIX/share/icons/hicolor/scalable/apps
	icon.files += ../dist/xdg/icons/scalable/e2-sat-editor.svg
	icon_256.path = $$PREFIX/share/icons/hicolor/256x256/apps
	icon_256.files += ../dist/xdg/icons/256x256/e2-sat-editor.png
	icon_128.path = $$PREFIX/share/icons/hicolor/128x128/apps
	icon_128.files += ../dist/xdg/icons/128x128/e2-sat-editor.png
	icon_64.path = $$PREFIX/share/icons/hicolor/64x64/apps
	icon_64.files += ../dist/xdg/icons/64x64/e2-sat-editor.png
	translations.path = $$PREFIX/share/e2-sat-editor/translations
	translations.files += ../res/locale/*.qm
	license.path = $$PREFIX/share/e2-sat-editor
	license.files += COPYING

	INSTALLS += target desktop icon icon_256 icon_128 icon_64 translations license
}

