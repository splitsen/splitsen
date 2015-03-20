# cmake -G "Visual Studio 12 2013"
# cmake -G "Visual Studio 14 2015"
#
# cmake --build . 
#
# cmake --build . --config Release

cmake_minimum_required (VERSION 3.2.1)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib_[CONFIG])
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib_[CONFIG])
#PROJECT_BINARY_DIR contains the full path to the top level directory of your build tree

#release build
#cmake -DCMAKE_CXX_FLAGS_RELEASE --build ./

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

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
