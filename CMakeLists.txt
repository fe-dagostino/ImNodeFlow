cmake_minimum_required(VERSION 3.26)

# CREATE PROJECT
project( ImNodeFlow
         VERSION 0.1.0
         DESCRIPTION "ImNodeFlow"
         LANGUAGES CXX C
)

add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

# SET SOURCE FILES FOR PROJECT
file(GLOB_RECURSE _HDRS "include/*.h")
file(GLOB_RECURSE _SRCS "src/*.cpp" "src/*.h" "src/*.inl")

# CREATE LIBRARY FROM SOURCE_FILES
add_library(ImNodeFlow ${_SRCS} ${_HDRS})

# FETCH DEPENDENCIES
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(FetchContent)
include(imgui)

# PREP TO USE "#include <>"
target_include_directories(ImNodeFlow PRIVATE ${imgui_SOURCE_DIR} 
                                              ${PROJECT_SOURCE_DIR}/include 
                                              ${PROJECT_SOURCE_DIR}/src 
                          )
target_include_directories(ImNodeFlow PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries( ImNodeFlow PRIVATE ImGui)