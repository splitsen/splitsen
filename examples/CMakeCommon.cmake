cmake_minimum_required (VERSION 3.2.1)
#
# generate VS projects
# 	cmake -G "Visual Studio 12 2013"
# 	cmake -G "Visual Studio 14 2015"
#
# debug build
# 	cmake --build . 
#
# release build
# 	cmake --build . --config Release
#

#PROJECT_BINARY_DIR contains the full path to the top level directory of your build tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Help on boost 
# http://www.cmake.org/cmake/help/v3.0/module/FindBoost.html
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
find_package(Boost
	1.57.0				# Minimum version
	REQUIRED            # Fail with error if Boost is not found
	COMPONENTS date_time	# Boost libraries by their canonical name
#	COMPONENTS system	# Boost libraries by their canonical name
  )

set(Splice_INCLUDE_DIRS $ENV{SPLICE_ROOT})
if(NOT Splice_INCLUDE_DIRS)
	message(FATAL_ERROR "SPLICE_ROOT must be defined")
endif()

include_directories(BEFORE ${Splice_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} )

link_directories(${Boost_LIBRARY_DIRS})
