project("log_test")

file(GLOB_RECURSE SRC "*.cpp" "*.h")
add_executable(${PROJECT_NAME} ${SRC})

if (UNIX)
  target_link_libraries(${PROJECT_NAME} log -pthread)
elseif(WIN32)
  target_link_libraries(${PROJECT_NAME} log)
endif()

