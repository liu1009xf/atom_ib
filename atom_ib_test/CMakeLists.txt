# define minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# turn on testing
enable_testing()

add_subdirectory(gtest)

include_directories(${PROJECT_SOURCE_DIR})