cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-unused-parameter -std=c++11")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
find_package (GSOAP REQUIRED)

add_library (cobaltcore SHARED
	libcobaltcore/bytestream.cc
	libcobaltcore/config.cc
	libcobaltcore/cmdlineparser.cc
	libcobaltcore/format.cc
	libcobaltcore/ip.cc
	libcobaltcore/string.cc
	libcobaltcore/tcp.cc
	libcobaltcore/time.cc
	libcobaltcore/udp.cc
	libcobaltcore/variant.cc
	libcobaltcore/misc.cc
	libcobaltcore/xml.cc
	libcobaltcore/xml_scanner.cc
)

add_executable (cobalt
	src/irc/factoids.cpp
	src/irc/irc.cpp
	src/irc/irc_channel.cpp
	src/irc/irc_commands.cpp
	src/irc/irc_user.cpp
	src/soap/soapC.c
	src/soap/soapClient.c
	src/3rdparty/huffman.cpp
	src/cobalt.cpp
	src/mantisbt.cpp
	src/utility.cpp
	src/zanserver.cpp
)

target_link_libraries (cobalt
	gsoap
	cobaltcore
)