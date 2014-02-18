cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-unused-parameter -std=c++0x")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_executable (cobalt
	src/cobalt.cpp
#	src/types/bytestream.cc
#	src/types/cmdlineparser.cc
	src/types/config.cc
	src/types/date.cc
	src/types/flood_throttle.cc
	src/types/format.cc
	src/types/ip.cc
	src/types/misc.cc
	src/types/stopwatch.cc
	src/types/string.cc
	src/types/string_list.cc
	src/types/tcp.cc
	src/types/time.cc
#	src/types/udp.cc
	src/types/variant.cc
	src/types/xml_document.cc
	src/types/xml_node.cc
	src/types/xml_scanner.cc
)
