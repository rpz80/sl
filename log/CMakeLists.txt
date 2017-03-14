cmake_minimum_required(VERSION 2.8)
project("log")

file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_library(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} sm)

