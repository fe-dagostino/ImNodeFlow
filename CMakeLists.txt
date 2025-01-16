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

# FETCH DEPENDENCIES
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(FetchContent)
include(imgui)

include_directories( ${imgui_SOURCE_DIR} ${imnodeflow_SOURCE_DIR}/include ${imnodeflow_SOURCE_DIR}/src )

# CREATE LIBRARY FROM SOURCE_FILES
add_library(ImNodeFlow ${_SRCS} ${_HDRS})

# PREP TO USE "#include <>"
target_include_directories(ImNodeFlow PRIVATE ${imgui_SOURCE_DIR} 
                                              ${PROJECT_SOURCE_DIR}/include 
                                              ${PROJECT_SOURCE_DIR}/src 
                          )
target_include_directories(ImNodeFlow PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries( ImNodeFlow PRIVATE ImGui)


if(hasParent)
  message( INFO " EXPORT ImNodeFlow SRC DIR: [${imnodeflow_SOURCE_DIR}]" )
  message( INFO " EXPORT ImNodeFlow BIN DIR: [${imnodeflow_BINARY_DIR}]" )
  # make both ${imnodeflow_SOURCE_DIR} and ${imnodeflow_BINARY_DIR} visible at parent scope.
  set( imnodeflow_SOURCE_DIR ${imnodeflow_SOURCE_DIR} PARENT_SCOPE )
  set( imnodeflow_BINARY_DIR ${imnodeflow_BINARY_DIR} PARENT_SCOPE )
endif(hasParent)