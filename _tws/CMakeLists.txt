# define minimum cmake version
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(tws C CXX)

file(GLOB_RECURSE CPP_FILES ${PROJECT_SOURCE_DIR}/tws/*.cpp)
add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME}
    PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME})

# target_include_directories(tws PRIVATE
#     ${PROJECT_SOURCE_DIR}
# )

# include_directories(${PROJECT_SOURCE_DIR})

set(${PROJECT_NAME}_INCLUDE_DIRS 
        ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)