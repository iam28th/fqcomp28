cmake_minimum_required(VERSION 3.26)
project(fqcomp28 LANGUAGES CXX C ASM)

# C++20 required (becuase of SeqAn3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vendor-specific extensions disabled
set(CMAKE_CXX_EXTENSIONS OFF)
# exports compile_commands.json - used by clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # TODO check its effect on performance
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ipo_supported})
endif()

add_subdirectory(src bin)
if (${BUILD_TESTING})
    include(CTest)
    add_subdirectory(test)
endif()
