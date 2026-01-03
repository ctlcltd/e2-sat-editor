VERSION = 1.8.1
QMAKE_TARGET_BUNDLE_PREFIX = io.github.ctlcltd
QMAKE_BUNDLE = e2se
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

RESOURCES += ../res/resources.qrc

QMAKE_CXXFLAGS += -std=c++17

macx {
	INCLUDEPATH += /usr/local/opt/curl/include
	LIBS += -L/usr/local/opt/curl/lib -lcurl
	HEADERS += gui/platforms/platform_macx.h
	SOURCES += gui/platforms/platform_macx.mm
	RESOURCES += ../res/resources-toolbar-macx.qrc
	TARGET = e2" "SAT" "Editor
} else {
	INCLUDEPATH += /usr/curl/include
	LIBS += -L/usr/curl/lib -lcurl
	RESOURCES += ../res/resources-toolbar.qrc
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
	metainfo.path = $$PREFIX/share/metainfo
	metainfo.files += ../dist/xdg/io.github.ctlcltd.e2se.desktop
	icon.path = $$PREFIX/share/icons/hicolor/scalable/apps
	icon.files += ../dist/xdg/icons/scalable/e2-sat-editor.svg
	icon_512.path = $$PREFIX/share/icons/hicolor/512x512/apps
	icon_512.files += ../dist/xdg/icons/512x512/e2-sat-editor.png
	icon_256.path = $$PREFIX/share/icons/hicolor/256x256/apps
	icon_256.files += ../dist/xdg/icons/256x256/e2-sat-editor.png
	icon_128.path = $$PREFIX/share/icons/hicolor/128x128/apps
	icon_128.files += ../dist/xdg/icons/128x128/e2-sat-editor.png
	icon_64.path = $$PREFIX/share/icons/hicolor/64x64/apps
	icon_64.files += ../dist/xdg/icons/64x64/e2-sat-editor.png
	icon_48.path = $$PREFIX/share/icons/hicolor/48x48/apps
	icon_48.files += ../dist/xdg/icons/48x48/e2-sat-editor.png
	icon_32.path = $$PREFIX/share/icons/hicolor/32x32/apps
	icon_32.files += ../dist/xdg/icons/32x32/e2-sat-editor.png
	translations.path = $$PREFIX/share/e2-sat-editor/translations
	translations.files += ../res/locale/*.qm
	license.path = $$PREFIX/share/e2-sat-editor
	license.files += LICENSE-GPL-3.0-or-later

	INSTALLS += target desktop metainfo icon icon_512 icon_256 icon_128 icon_64 icon_48 icon_32 translations license
}

