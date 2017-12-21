cmake_minimum_required(VERSION 2.8.4)
project(BlockGL)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# GLFW
find_package(GLFW3 REQUIRED)

# GLAD
add_library(GLAD STATIC lib/glad/src/glad.c)
target_include_directories(GLAD PUBLIC "${CMAKE_SOURCE_DIR}/lib/glad/include")

# GLM
find_package(GLM REQUIRED)

set(SOURCE_FILES src/main.c)
add_executable(BlockGL ${SOURCE_FILES})

target_link_libraries(BlockGL glfw GLAD glm)
