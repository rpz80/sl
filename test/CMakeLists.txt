project("log_test")

find_package (Threads)

file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} log)
