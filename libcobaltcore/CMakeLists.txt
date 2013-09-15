cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-unused-parameter -std=c++11")
endif()

add_library (cobaltcore SHARED
	bytestream.cc
	config.cc
	cmdlineparser.cc
	format.cc
	ip.cc
	string.cc
	tcp.cc
	time.cc
	udp.cc
	variant.cc
	misc.cc
	xml.cc
	xml_scanner.cc
)
