cmake_minimum_required(VERSION 3.20)

project(e2se-cli VERSION 1.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES
	../logger/logger.cpp
	../e2db/e2db_abstract.cpp
	../e2db/e2db_converter.cpp
	../e2db/e2db_maker.cpp
	../e2db/e2db_parser.cpp
	../e2db/e2db.cpp
	e2db_termctl.cpp
	e2db_cli.cpp
	main.cpp
)

add_executable(e2se-cli ${SOURCES})

if(APPLE)
	install(TARGETS e2se-cli DESTINATION .)
else()
	install(TARGETS e2se-cli)
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

configure_file("../../dist/common/Readme.txt.in" "Readme.txt" @ONLY NEWLINE_STYLE ${NEWLINE})
configure_file("../COPYING" "License.txt" @ONLY NEWLINE_STYLE ${NEWLINE})

if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Readme.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2se-cli)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/License.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2se-cli)
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
