include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

add_library(fqzcomp28Core OBJECT
    app.cpp
    app.h
    process.cpp
    process.h
    settings.h
)


target_link_libraries(fqzcomp28Core PRIVATE 
    seqan3::seqan3 
    CLI11::CLI11
    libzstd_static
)

# mark SeqAn3 include folder as 'system', so that its headers won't yield warnings  
target_include_directories(fqzcomp28Core SYSTEM PRIVATE 
    $<TARGET_PROPERTY:seqan3::seqan3,INTERFACE_INCLUDE_DIRECTORIES>
)

target_compile_definitions(fqzcomp28Core PRIVATE SEQAN3_DISABLE_NEWER_COMPILER_DIAGNOSTIC)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") 
    target_compile_options(fqzcomp28Core PRIVATE 
        -Wall
        -Wconversion
        -Wsign-conversion
        -Wold-style-cast
        -Wcast-align
        -Wextra
        -frecord-gcc-switches  # to retain certain flags in the executable
        -g3
        $<$<CONFIG:DEBUG>:-O0>
    )
    target_link_options(fqzcomp28Core PUBLIC
        -frecord-gcc-switches
    )
endif()

add_executable(fqzcomp28 bootstrap.cpp)
target_link_libraries(fqzcomp28 PRIVATE fqzcomp28Core)
