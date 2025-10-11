cmake_minimum_required(VERSION 3.14)
project(
  Nichess
  VERSION 0.1
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wshadow -march=native -pedantic")
add_library(
  nichess SHARED
  src/nichess.cpp
  src/util.cpp
  src/zobrist.cpp
  src/gamecache.cpp
  include/nichess/nichess.hpp
  include/nichess/util.hpp
  include/nichess/constants.hpp
  include/nichess/zobrist.hpp
  include/nichess/gamecache.hpp
  )
target_include_directories(nichess PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(nichess PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif()
