cmake_minimum_required (VERSION 3.2.1)
include(../../CMakeCommon.cmake)

find_package(Boost
	COMPONENTS serialization	# Boost libraries by their canonical name
  )

add_executable (multi_server main.cpp my_logger.cpp echo_message.cpp)

