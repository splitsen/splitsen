cmake_minimum_required (VERSION 3.2.1)

project (splice_examples)

cmake_minimum_required (VERSION 3.2.1)
#
# generate
#   cmake .
# 	VS only: cmake -G "Visual Studio 12 2013"
#
# debug build
# 	cmake --build . 
#   VS only: msbuild splice_examples.sln /maxcpucount:4
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
	COMPONENTS system	# Boost libraries by their canonical name
  )

set(Splice_INCLUDE_DIRS $ENV{SPLICE_ROOT})
if(NOT Splice_INCLUDE_DIRS)
	message(FATAL_ERROR "SPLICE_ROOT must be defined")
endif()

if ( CMAKE_COMPILER_IS_GNUCXX )

	# # # Warning removal is made only with VS2010 (and disabled for GCC4.6.3)
	# # #-w Inhibit all warning messages. 

	# # set(CMAKE_CXX_FLAGS "${MY_CMAKE_FLAGS} ${CMAKE_CXX_FLAGS} -w")

	# # find_package (Threads)

	# # set (ADDITIONAL_LIBRARIES
		# # ${CMAKE_DL_LIBS}
		# # ${CMAKE_THREAD_LIBS_INIT}
		# # rt
		# # )

    # # link_directories(/usr/lib/)

else()
	# set(CMAKE_CXX_FLAGS "${MY_CMAKE_FLAGS} ")
	# disable default handling of unicode
	add_definitions(-DUINCODE -D_UNICODE -U_MBCS)
endif ()

message(STATUS "Splice_INCLUDE_DIRS: " ${Splice_INCLUDE_DIRS} )
message(STATUS "Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
include_directories(BEFORE 
	${Splice_INCLUDE_DIRS} 
	${Boost_INCLUDE_DIRS} 
	)

link_directories(BEFORE  
	${Boost_LIBRARY_DIRS}
	)
  
# Recurse into subdirectories
#add_subdirectory (./splice)
add_subdirectory (./json_server)
add_subdirectory (./mono_protocol_echo/echo_client)
add_subdirectory (./mono_protocol_echo/echo_ws_server)
add_subdirectory (./mono_protocol_echo/echo_server)
add_subdirectory (./multi_protocol_echo/raw_client)
add_subdirectory (./multi_protocol_echo/serialize_client)
add_subdirectory (./multi_protocol_echo/server)

