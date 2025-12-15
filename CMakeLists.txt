cmake_minimum_required(VERSION 4.2.0)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD
  # Check CMake's `Help/dev/experimental.rst` for value corresponding to your cmake verison
  "d0edc3af-4c50-42ea-a356-e2862fe7a444")
set(CMAKE_CXX_MODULE_STD 1)

project(cpptail)

set(CMAKE_CXX_STANDARD 23)

add_executable(cpptail src/main.cpp)
