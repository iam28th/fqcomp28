cmake_minimum_required(VERSION 3.26)
project(fqcomp28 LANGUAGES CXX C ASM)

# C++20 required (becuase of SeqAn3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vendor-specific extensions disabled
set(CMAKE_CXX_EXTENSIONS OFF)
# exports compile_commands.json - used by clang tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src bin)
if (${BUILD_TESTING})
    include(CTest)
    add_subdirectory(test)
endif()
