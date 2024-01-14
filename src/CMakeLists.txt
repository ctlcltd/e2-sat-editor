cmake_minimum_required(VERSION 3.16)

project(e2-sat-editor VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(WITH_QT5 "build requiring Qt5")
option(WITHOUT_CLI "make the bundle without e2se-cli")

if(WITH_QT5)
	set(USE_QT_NAMES Qt5)
else()
	set(USE_QT_NAMES Qt6)
endif()

find_package(QT NAMES ${USE_QT_NAMES} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)
find_package(CURL)

set(SOURCES
	logger/logger.cpp
	e2db/e2db_abstract.cpp
	e2db/e2db_converter.cpp
	e2db/e2db_maker.cpp
	e2db/e2db_parser.cpp
	e2db/e2db.cpp
	ftpcom/ftpcom.cpp
	gui/platforms/platform.h
	gui/toolkit/InputControlEditEventObserver.cpp
	gui/toolkit/ListIconDragDropEventHandler.cpp
	gui/toolkit/ListProxyStyle.h
	gui/toolkit/TabBarProxyStyle.h
	gui/toolkit/ThemeChangeEventObserver.h
	gui/toolkit/TreeDragDropEventHandler.cpp
	gui/toolkit/TreeDropIndicatorEventPainter.cpp
	gui/toolkit/TreeEventHandler.cpp
	gui/toolkit/TreeItemChangedEventObserver.cpp
	gui/toolkit/TreeProxyStyle.h
	gui/toolkit/WidgetWithBackdrop.cpp
	gui/about.cpp
	gui/channelBookView.cpp
	gui/dataHandler.cpp
	gui/dialAbstract.cpp
	gui/dialChannelBook.cpp
	gui/e2db_gui.cpp
	gui/editBouquet.cpp
	gui/editMarker.cpp
	gui/editService.cpp
	gui/editTransponder.cpp
	gui/editTunersets.cpp
	gui/editTunersetsTable.cpp
	gui/editTunersetsTransponder.cpp
	gui/editUserbouquet.cpp
	gui/ftpcom_gui.cpp
	gui/ftpHandler.cpp
	gui/gui.cpp
	gui/l10n.h
	gui/mainView.cpp
	gui/piconsView.cpp
	gui/printable.cpp
	gui/settings.cpp
	gui/tab.cpp
	gui/theme.cpp
	gui/tools.cpp
	gui/transpondersView.cpp
	gui/tunersetsView.cpp
	gui/viewAbstract.cpp
	main.cpp
	../res/resources.qrc
)

if(APPLE)
	list(APPEND SOURCES gui/platforms/platform_macx.mm)
	list(APPEND SOURCES ../res/resources-toolbar-macx.qrc)
else()
	list(APPEND SOURCES ../res/resources-toolbar.qrc)
endif()


if(WIN32)
	ENABLE_LANGUAGE(RC)
	# ENABLE_LANGUAGE(RC OPTIONAL)
endif()

set(app_icon_resource_windows "../dist/windows/e2-sat-editor.rc")
list(APPEND SOURCES ${app_icon_resource_windows})

set(MACOSX_BUNDLE_BUNDLE_NAME "e2 SAT Editor")
set(MACOSX_BUNDLE_BUNDLE_VERSION 1.1)
set(MACOSX_BUNDLE_ICON_FILE e2-sat-editor.icns)

set(app_icon_macos "../dist/macos/e2-sat-editor.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
	MACOSX_PACKAGE_LOCATION "Resources")
list(APPEND SOURCES ${app_icon_resource_windows})

if(NOT WITHOUT_CLI)
	add_subdirectory(cli)
endif()

add_executable(e2-sat-editor WIN32 MACOSX_BUNDLE ${SOURCES})

target_link_libraries(e2-sat-editor PUBLIC
	Qt::Core
	Qt::Widgets
	Qt::PrintSupport
	CURL::libcurl
)

install(TARGETS e2-sat-editor)

if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	install(FILES "../dist/xdg/e2-sat-editor.desktop"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	if(NOT WITHOUT_CLI)
		install(FILES "../dist/xdg/e2se-cli.desktop"
				DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	endif()
	install(FILES "../dist/xdg/io.github.ctlcltd.e2se.metainfo.xml"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)

	install(FILES "../dist/xdg/icons/scalable/e2-sat-editor.svg"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
	install(FILES "../dist/xdg/icons/256x256/e2-sat-editor.png"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
	install(FILES "../dist/xdg/icons/128x128/e2-sat-editor.png"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)
	install(FILES "../dist/xdg/icons/128x128/e2-sat-editor.png"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

	install(DIRECTORY "../dist/translations" DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor
			FILES_MATCHING PATTERN "*.qm")

	install(FILES "COPYING"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor)
endif()

