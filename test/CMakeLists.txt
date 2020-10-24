cmake_minimum_required(VERSION 3.4)

set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_library(TestPass SHARED
    pass.cpp
)

set_target_properties(TestPass PROPERTIES
    COMPILE_FLAGS "-std=c++14 -fno-rtti"
)
add_custom_command(TARGET TestPass POST_BUILD COMMAND bash run.sh)
