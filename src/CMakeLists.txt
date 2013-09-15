cmake_minimum_required (VERSION 2.8)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-unused-parameter -std=c++11")
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..)
find_package (GSOAP REQUIRED)

add_executable (cobalt
	irc/irc.cpp
	irc/irc_channel.cpp
	irc/irc_commands.cpp
	irc/irc_user.cpp
	soap/soapC.c
	soap/soapClient.c
	3rdparty/huffman.cpp
	cobalt.cpp
	mantisbt.cpp
	utility.cpp
	zanserver.cpp
)

target_link_libraries (cobalt
	gsoap
	cobaltcore
)
