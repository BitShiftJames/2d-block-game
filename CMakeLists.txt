cmake_minimum_required(VERSION 4.1.1)
project(Lumaria)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(${PROJECT_NAME} src/main.cpp)


add_subdirectory(external/raylib)

target_link_libraries(${PROJECT_NAME} raylib)
