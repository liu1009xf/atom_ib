# define minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(atom_ib_api C CXX)

file(GLOB_RECURSE CPP_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${tws_INCLUDE_DIRS})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
    PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} tws)

target_include_directories(atom_ib_api PRIVATE
    ${PROJECT_SOURCE_DIR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(${PROJECT_NAME}_INCLUDE_DIRS 
        ${PROJECT_SOURCE_DIR}
        ${tws_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)