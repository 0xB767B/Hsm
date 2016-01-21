cmake_minimum_required(VERSION 3.3)
project(Hsm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    hsm.h
    main.cpp
    README.md)

add_executable(Hsm ${SOURCE_FILES})