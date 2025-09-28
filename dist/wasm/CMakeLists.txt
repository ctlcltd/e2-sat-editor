cmake_minimum_required(VERSION 3.22)

project(e2-sat-editor VERSION 1.8.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

set(SOURCES
	logger/logger.cpp
	e2db/e2db_abstract.cpp
	e2db/e2db_converter.cpp
	e2db/e2db_maker.cpp
	e2db/e2db_parser.cpp
	e2db/e2db_utils.cpp
	e2db/e2db.cpp
	gui/platforms/platform.h
	gui/toolkit/InputControlEditEventObserver.cpp
	gui/toolkit/ListIconDragDropEventHandler.cpp
	gui/toolkit/ListProxyStyle.h
	gui/toolkit/TabBarProxyStyle.h
	gui/toolkit/TreeDragDropEventHandler.cpp
	gui/toolkit/TreeDropIndicatorEventPainter.cpp
	gui/toolkit/TreeEventHandler.cpp
	gui/toolkit/TreeItemChangedEventObserver.cpp
	gui/toolkit/TreeProxyStyle.h
	gui/toolkit/WidgetBackdrop.cpp
	gui/toolkit/WidgetEventHandler.cpp
	gui/about.cpp
	gui/channelBookView.cpp
	gui/connectionPresets.cpp
	gui/dataHandler.cpp
	gui/dialAbstract.cpp
	gui/dialChannelBook.cpp
	gui/dialConvertM3u.cpp
	gui/e2db_gui.cpp
	gui/editBouquet.cpp
	gui/editFavourite.cpp
	gui/editMarker.cpp
	gui/editService.cpp
	gui/editTransponder.cpp
	gui/editTunersets.cpp
	gui/editTunersetsTable.cpp
	gui/editTunersetsTransponder.cpp
	gui/editUserbouquet.cpp
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
	../res/resources-toolbar.qrc
)

qt_add_resources(SOURCES ../res/enigma_db.qrc OPTIONS -no-compress)


qt_add_executable(e2-sat-editor ${SOURCES})

target_link_options(e2-sat-editor PUBLIC -sASYNCIFY -Os)

target_link_libraries(e2-sat-editor PUBLIC
	Qt::Core
	Qt::Widgets
	Qt::PrintSupport
)

