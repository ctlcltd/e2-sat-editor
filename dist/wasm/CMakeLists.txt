cmake_minimum_required(VERSION 3.16)

project(e2-sat-editor VERSION 0.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(WITH_QT5 "build requiring Qt5")

if(WITH_QT5)
	set(USE_QT_NAMES Qt5)
else()
	set(USE_QT_NAMES Qt5 Qt6)
endif()

find_package(QT NAMES ${USE_QT_NAMES} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets PrintSupport)
# find_package(CURL)

set(SOURCES
	logger/logger.cpp
	e2db/e2db_abstract.cpp
	e2db/e2db_converter.cpp
	e2db/e2db_maker.cpp
	e2db/e2db_parser.cpp
	e2db/e2db.cpp
#  	ftpcom/ftpcom.cpp
	gui/platforms/platform.h
	gui/toolkit/InputControlEditEventObserver.cpp
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
	gui/mainView.cpp
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
endif()


if(WIN32)
	ENABLE_LANGUAGE(RC)
	# ENABLE_LANGUAGE(RC OPTIONAL)
endif()

set(app_icon_resource_windows "../dist/windows/e2-sat-editor.rc")
list(APPEND SOURCES ${app_icon_resource_windows})

set(MACOSX_BUNDLE_BUNDLE_NAME "e2 SAT Editor")
set(MACOSX_BUNDLE_BUNDLE_VERSION 0.6)
set(MACOSX_BUNDLE_ICON_FILE e2-sat-editor.icns)

set(app_icon_macos "../dist/macos/e2-sat-editor.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
	MACOSX_PACKAGE_LOCATION "Resources")
list(APPEND SOURCES ${app_icon_resource_windows})

qt_add_resources(SOURCES ../res/enigma_db.qrc OPTIONS -no-compress)


qt_add_executable(e2-sat-editor WIN32 MACOSX_BUNDLE ${SOURCES})

target_link_options(e2-sat-editor PUBLIC -sASYNCIFY -Os)

target_link_libraries(e2-sat-editor PUBLIC
	Qt::Core
	Qt::Gui
	Qt::Widgets
	Qt::PrintSupport
# 	CURL::libcurl
)

