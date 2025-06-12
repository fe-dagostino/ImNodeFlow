cmake_minimum_required(VERSION 3.26)

# CREATE PROJECT
project( ImNodeFlow
         VERSION 0.1.0
         DESCRIPTION "ImNodeFlow"
         LANGUAGES CXX C
)

# uncomment to set a default CXX standard for the external tools like clang-tidy and cppcheck
# and the targets that do not specify a standard.
# If not set, the latest supported standard for your compiler is used
# You can later set fine-grained standards for each target using `target_compile_features`
# Note: linking together projects compiled with different C++ standards may work, but
# it is not recommended because of possible issues with ABI
set(CMAKE_CXX_STANDARD 20)
  
# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS           ON)
set(CMAKE_CXX_STANDARD_REQUIRED    ON)
set(CMAKE_C_STANDARD_REQUIRED      ON)

add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

# SET SOURCE FILES FOR PROJECT
file(GLOB_RECURSE _HDRS "include/*.h")
file(GLOB_RECURSE _SRCS "src/*.cpp" "src/*.h" "src/*.inl")

# FETCH DEPENDENCIES
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(FetchContent)
include(imgui)

# CREATE LIBRARY FROM SOURCE_FILES
add_library(ImNodeFlow ${_SRCS} ${_HDRS})

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src )
