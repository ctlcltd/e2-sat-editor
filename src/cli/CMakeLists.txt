cmake_minimum_required(VERSION 3.16)

project(e2db-cli VERSION 0.6 LANGUAGES CXX)

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
	e2db-cli.cpp
	main.cpp
)

add_executable(e2se-cli ${SOURCES})

