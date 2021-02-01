cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

include_directories(${atom_ib_api_INCLUDE_DIRS})

include("${PROJECT_SOURCE_DIR}/others/cmake/download_project.cmake")
#
# external: gtest
#
download_project(
    PROJ googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG master
    UPDATE_DISCONNECTED 1
)
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

file(GLOB CPP_FILES ${PROJECT_SOURCE_DIR}/atom_ib_test/gtest/*.cpp)

message("Test Includes: " ${CPP_FILES})
add_executable(atom_test ${CPP_FILES})

target_sources(atom_test
    PRIVATE
        ${CPP_FILES}
)

target_link_libraries(atom_test
    PRIVATE
        gtest_main
        atom_ib_api
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Prevent GoogleTest from using PThreads
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

# turn on testing
enable_testing()
# define test

add_test(
    NAME core_test
    COMMAND $<TARGET_FILE:atom_test>
)