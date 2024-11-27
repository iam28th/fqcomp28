cmake_minimum_required(VERSION 3.26)
project(fqzcomp28)

# C++20 required (cause I plan to use SeqAn3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# vendor-specific extensions disabled
set(CMAKE_CXX_EXTENSIONS OFF)

# link-time opimization enabled when available
# TODO: add a switch to disable; once done, check its effect on performance
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ipo_supported})

add_executable(fqzcomp28 main.cpp)
