cmake_minimum_required(VERSION 3.20)

project(e2se-cli VERSION 1.4.0 LANGUAGES CXX)

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

if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)

	install(FILES "Readme.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2se-cli/readme)
	install(FILES "License.txt"
			DESTINATION ${CMAKE_INSTALL_DATADIR}/e2se-cli/copyright)
elseif(APPLE)
	install(FILES "Readme.txt"
			DESTINATION .)
	install(FILES "License.txt"
			DESTINATION .)
else()
	install(FILES "Readme.txt"
			DESTINATION bin)
	install(FILES "License.txt"
			DESTINATION bin)
endif()
