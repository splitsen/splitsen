cmake_minimum_required (VERSION 3.2.1)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 

find_package(Boost
	1.57.0				# Minimum version
	REQUIRED            # Fail with error if Boost is not found
	COMPONENTS date_time	# Boost libraries by their canonical name
#	COMPONENTS system	# Boost libraries by their canonical name
  )

  #include_directories(BEFORE ./../../.. ${Boost_INCLUDE_DIRS} )
include_directories(BEFORE ./.. ${Boost_INCLUDE_DIRS} )

link_directories(${Boost_LIBRARY_DIRS})
