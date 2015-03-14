cmake_minimum_required (VERSION 3.2.1)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

#Multi-threaded Debug (/MTd)

find_package(Boost
	1.57.0				# Minimum version
	REQUIRED            # Fail with error if Boost is not found
	COMPONENTS date_time	# Boost libraries by their canonical name
#	COMPONENTS system	# Boost libraries by their canonical name
  )

include("./../../common.cmake")

include_directories(BEFORE ./../../.. ${Boost_INCLUDE_DIRS} )

link_directories(${Boost_LIBRARY_DIRS})

#link_libraries(echo_client ${Boost_LIBRARIES})

add_executable (echo_client client.cpp)

