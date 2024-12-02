cmake_minimum_required(VERSION 3.26)
project(fqzcomp28 LANGUAGES CXX)

# C++20 required (becuase of SeqAn3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vendor-specific extensions disabled
set(CMAKE_CXX_EXTENSIONS OFF)
# exports compile_commands.json - used by clangd 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# link-time opimization enabled when available
# TODO: add a switch to disable; once done, check its effect on performance
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ipo_supported})

include(${CMAKE_SOURCE_DIR}/../cmake/Dependencies.cmake)

add_executable(fqzcomp28 
    app.cpp
    app.h
    main.cpp
    process.cpp
    process.h
    settings.h
)

target_link_libraries(fqzcomp28 PRIVATE 
    seqan3::seqan3 
    CLI11::CLI11
)

target_compile_options(fqzcomp28 PRIVATE -DSEQAN3_DISABLE_NEWER_COMPILER_DIAGNOSTIC)
