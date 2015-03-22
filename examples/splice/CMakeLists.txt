cmake_minimum_required (VERSION 3.2.1)
include(../CMakeCommon.cmake)

add_library(splice ../../splice/src.cpp ../../splice/tcp_session.hpp)
