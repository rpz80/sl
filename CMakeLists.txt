cmake_minimum_required(VERSION 2.8)
project("log_root")

include(ExternalProject)

ExternalProject_Add(
  sm
  GIT_REPOSITORY "https://github.com/rpz80/sm.git"
  GIT_TAG "master"
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/../sm"
  INSTALL_COMMAND cmake -E echo "Skipping install step"
)

set(SM_INCLUDE_PATH "../sm")
set(INCLUDES "${CMAKE_SOURCE_DIR}" "${SM_INCLUDE_PATH}")

include_directories(${INCLUDES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

set(CMAKE_CXX_FLAGS "-std=c++1y")

add_subdirectory("test")
add_subdirectory("log")
