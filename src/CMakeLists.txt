cmake_minimum_required(VERSION 3.16)
project(e2-sat-editor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)
find_package(CURL)

qt_add_executable(e2-sat-editor WIN32 MACOSX_BUNDLE
	e2db.cpp
	ftpcom.cpp
	gui/about.cpp
	gui/channelBook.cpp
	gui/DropEventHandler.h
	gui/e2db_gui.cpp
	gui/editService.cpp
	gui/gui.cpp
	gui/settings.cpp
	gui/tab.cpp
	gui/TreeEventObserver.h
	gui/WidgetWithBackdrop.cpp
	main.cpp
)

target_link_libraries(e2-sat-editor PUBLIC
	Qt::Core
	Qt::Gui
	Qt::Widgets
	CURL::libcurl
)

#target_compile_options(e2-sat-editor PRIVATE 
#    cxx_std_17
#)


# Resources:
set(resources_resource_files
	"../res/dark/add.png"
	"../res/dark/close.png"
	"../res/dark/delete.png"
	"../res/dark/edit.png"
	"../res/dark/export.png"
	"../res/dark/file-new.png"
	"../res/dark/file-open.png"
	"../res/dark/file.png"
	"../res/dark/import.png"
	"../res/dark/remove.png"
	"../res/dark/round-checkmark.png"
	"../res/dark/round-delete.png"
	"../res/dark/round-info.png"
	"../res/dark/round-question.png"
	"../res/dark/round-warning.png"
	"../res/dark/save.png"
	"../res/dark/search.png"
	"../res/dark/settings.png"
	"../res/e2-sat-editor.png"
	"../res/light/add.png"
	"../res/light/close.png"
	"../res/light/delete.png"
	"../res/light/edit.png"
	"../res/light/export.png"
	"../res/light/file-new.png"
	"../res/light/file-open.png"
	"../res/light/file.png"
	"../res/light/import.png"
	"../res/light/remove.png"
	"../res/light/round-checkmark.png"
	"../res/light/round-delete.png"
	"../res/light/round-info.png"
	"../res/light/round-question.png"
	"../res/light/round-warning.png"
	"../res/light/save.png"
	"../res/light/search.png"
	"../res/light/settings.png"
)

qt6_add_resources(e2-sat-editor "resources"
	PREFIX
		"/icons"
	BASE
		"../res"
	FILES
		${resources_resource_files}
)