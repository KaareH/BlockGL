cmake_minimum_required(VERSION 2.8.4)
project(BlockGL)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# GLFW
find_package(glfw3 REQUIRED)

# GLAD
add_library(GLAD STATIC lib/glad/src/glad.c)
target_include_directories(GLAD PUBLIC "${CMAKE_SOURCE_DIR}/lib/glad/include")

# Linmath
add_library(linmath INTERFACE IMPORTED)
set_target_properties(linmath PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/linmath/include")

# STB image
add_library(stb INTERFACE IMPORTED)
set_target_properties(stb PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/stb/include")

set(SOURCE_FILES src/main.c src/blockgl.h)
add_executable(BlockGL ${SOURCE_FILES})

target_link_libraries(BlockGL glfw GLAD linmath stb)

file(COPY "${PROJECT_SOURCE_DIR}/resources" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
