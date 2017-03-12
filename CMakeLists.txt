cmake_minimum_required(VERSION 2.8)
project("log_root")

include(ExternalProject)

ExternalProject_Add(
  sm

  GIT_REPOSITORY "https://github.com/rpz80/sm.git"
  GIT_TAG "master"
  
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/../sm"
  
  TEST_COMMAND ""
)

set(SM_INCLUDE_PATH "sm")
set(INCLUDES "${CMAKE_SOURCE_DIR}" "${SM_INCLUDE_PATH}")

include_directories(${INCLUDES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-std=c++1y")

add_subdirectory("test")
add_subdirectory("log")
