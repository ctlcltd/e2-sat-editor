cmake_minimum_required(VERSION 3.16)

project(e2se-cli VERSION 1.3.0 LANGUAGES CXX)

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

