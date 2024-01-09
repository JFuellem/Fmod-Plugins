
#CMAKE_HOST_SYSTEM_NAME

cmake_minimum_required(VERSION 3.10.0)

project(JRFBitcrush)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} SHARED ../fmod_bitcrush.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../../inc)





