
find_package(Boost
	COMPONENTS serialization	# Boost libraries by their canonical name
  )

add_executable (serialize_client 
	serialize_client.cpp 
	../server/echo_message.cpp
	)

