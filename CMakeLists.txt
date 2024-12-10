cmake_minimum_required(VERSION 3.26)
project(fqzcomp28 LANGUAGES CXX C ASM)

# C++20 required (becuase of SeqAn3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vendor-specific extensions disabled
set(CMAKE_CXX_EXTENSIONS OFF)
# exports compile_commands.json - used by clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# link-time opimization enabled when available
# TODO: add a switch to disable; once done, check its effect on performance
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ipo_supported})

include(CTest)
add_subdirectory(src bin)
add_subdirectory(test)
