cmake_minimum_required(VERSION 3.20)

project(e2-sat-editor VERSION 1.7.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(WITH_QT5 "build requiring Qt5")
option(WITHOUT_CLI "make the bundle without e2se-cli")
option(DOWNLOAD_CURL_CABUNDLE "download cabundle from curl website")

option(NOAUTODISCOVER "turn off auto-discover build directives")
option(NOCHECKUPDATE "turn off check update")
set(RELEASE_MEDIUM "" CACHE STRING "set the release medium")

if(WITH_QT5)
	set(USE_QT_NAMES Qt5)
else()
	set(USE_QT_NAMES Qt6)
endif()

if(NOAUTODISCOVER)
	add_compile_definitions(NOAUTODISCOVER)
endif()
if(NOCHECKUPDATE)
	add_compile_definitions(NOCHECKUPDATE)
endif()
if(RELEASE_MEDIUM)
	if(RELEASE_MEDIUM STREQUAL "aur")
		add_compile_definitions(RELEASE_MEDIUM RELEASE_MEDIUM_AUR)
	elseif(RELEASE_MEDIUM STREQUAL "flathub")
		add_compile_definitions(RELEASE_MEDIUM RELEASE_MEDIUM_FLATHUB)
	elseif(RELEASE_MEDIUM STREQUAL "github")
		add_compile_definitions(RELEASE_MEDIUM RELEASE_MEDIUM_GITHUB)
	elseif(RELEASE_MEDIUM STREQUAL "snapstore")
		add_compile_definitions(RELEASE_MEDIUM RELEASE_MEDIUM_SNAPSTORE)
	endif()
endif()

find_package(QT NAMES ${USE_QT_NAMES} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)
find_package(CURL)

set(SOURCES
	logger/logger.cpp
	e2db/e2db_abstract.cpp
	e2db/e2db_utils.cpp
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

if(NOT NOCHECKUPDATE)
	list(APPEND SOURCES gui/checkUpdate.cpp)
endif()

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

set(app_icon_macos "../dist/macos/e2-sat-editor.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES
	MACOSX_PACKAGE_LOCATION "Resources")
list(APPEND SOURCES ${app_icon_macos})

if(NOT WITHOUT_CLI)
	add_subdirectory(cli)
endif()

if(DOWNLOAD_CURL_CABUNDLE)
	file(DOWNLOAD https://curl.se/ca/cacert-2024-07-02.pem
		"curl-cacert.pem"
		EXPECTED_HASH SHA256=1bf458412568e134a4514f5e170a328d11091e071c7110955c9884ed87972ac9
	)

	if(APPLE)
		set(curl_cabundle_macos "curl-cacert.pem")
		set_source_files_properties(${curl_cabundle_macos} PROPERTIES
		MACOSX_PACKAGE_LOCATION "Resources")
		list(APPEND SOURCES ${curl_cabundle_macos})
	endif()
endif()

add_executable(e2-sat-editor WIN32 MACOSX_BUNDLE ${SOURCES})

target_link_libraries(e2-sat-editor PUBLIC
	Qt::Core
	Qt::Widgets
	Qt::PrintSupport
	CURL::libcurl
)

if(APPLE)
	set_target_properties(e2-sat-editor PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/../dist/macos/Info.plist.in
		OUTPUT_NAME "e2 SAT Editor"
	)

	install(TARGETS e2-sat-editor BUNDLE DESTINATION .)
else()
	install(TARGETS e2-sat-editor)
endif()

set(NEWLINE LF)
set(REPOSITORY_RES
	"https://github.com/ctlcltd/e2-sat-editor/raw/main/res")

if(WIN32)
	set(NEWLINE CRLF)
	set(SCREENSHOT_LIGHT "${REPOSITORY_RES}/screenshot-wlw.webp")
	set(SCREENSHOT_DARK "${REPOSITORY_RES}/screenshot-wdw.webp")
elseif(APPLE)
	set(SCREENSHOT_LIGHT "${REPOSITORY_RES}/screenshot-mlm.webp")
	set(SCREENSHOT_DARK "${REPOSITORY_RES}/screenshot-mdm.webp")
else()
	set(SCREENSHOT_LIGHT "${REPOSITORY_RES}/screenshot-flf.webp")
	set(SCREENSHOT_DARK "${REPOSITORY_RES}/screenshot-fdf.webp")
endif()

configure_file("../dist/common/Readme.txt.in" "Readme.txt" @ONLY NEWLINE_STYLE ${NEWLINE})
configure_file("COPYING" "License.txt" @ONLY NEWLINE_STYLE ${NEWLINE})

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
	install(FILES "../dist/xdg/icons/64x64/e2-sat-editor.png"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

	install(DIRECTORY "../dist/translations"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor
			FILES_MATCHING PATTERN "*.qm")

	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Readme.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/License.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor)
elseif(APPLE)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Readme.txt"
			DESTINATION .)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/License.txt"
			DESTINATION .)
else()
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Readme.txt"
			DESTINATION bin)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/License.txt"
			DESTINATION bin)
endif()

if(DOWNLOAD_CURL_CABUNDLE)
	if(UNIX AND NOT APPLE)
		install(FILES "curl-cacert.pem"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2-sat-editor)
	elseif(NOT APPLE)
		install(FILES "curl-cacert.pem"
			DESTINATION bin/ca-certificates)
	endif()
endif()

