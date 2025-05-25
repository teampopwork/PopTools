cmake_minimum_required(VERSION 3.10.0)
project(PopPak VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PopPak src/poppak.cpp src/pakinterface.cpp src/pakinterface.hpp src/common.cpp src/common.hpp)

