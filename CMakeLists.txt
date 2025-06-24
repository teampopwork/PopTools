cmake_minimum_required(VERSION 3.10.0)
project(PopTools VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/tinyxml2 EXCLUDE_FROM_ALL)
add_subdirectory(external/zlib EXCLUDE_FROM_ALL)
include_directories(external/zlib)
include_directories(external)
add_subdirectory(src)