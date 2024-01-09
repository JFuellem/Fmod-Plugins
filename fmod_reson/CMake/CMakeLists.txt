
#CMAKE_HOST_SYSTEM_NAME

cmake_minimum_required(VERSION 3.10.0)

project(JRFResonToBe)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} SHARED ../jrf_resonToBe_fmod.cpp ../jrf_reson.cpp ../jrf_reson.hpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../../inc)





