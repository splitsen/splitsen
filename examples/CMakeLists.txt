cmake_minimum_required (VERSION 3.2.1)

project (examples)
  
# Recurse into subdirectories
add_subdirectory (./splice)
add_subdirectory (./json_server)
add_subdirectory (./mono_protocol_echo/echo_client)
add_subdirectory (./mono_protocol_echo/echo_ws_server)
add_subdirectory (./mono_protocol_echo/echo_server)
add_subdirectory (./multi_protocol_echo/raw_client)
add_subdirectory (./multi_protocol_echo/serialize_client)
add_subdirectory (./multi_protocol_echo/server)

