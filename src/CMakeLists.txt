include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

add_library(fqzcomp28Core OBJECT
    app.cpp
    archive.cpp
    compressed_buffers.cpp
    encoding_context.cpp
    memcompress.cpp
    fastq_io.cpp
    headers.cpp
    prepare.cpp
    process.cpp
    report.cpp
)

target_include_directories(fqzcomp28Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(fqzcomp28Core PRIVATE 
    CLI11::CLI11
    libzstd_static
)

# mark include folders as 'system', so that the headers don't yield warnings  
target_include_directories(fqzcomp28Core SYSTEM PRIVATE 
    $<TARGET_PROPERTY:libzstd_static,INTERFACE_INCLUDE_DIRECTORIES>
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") 
    # public so that fqzcomp28 executable and unit_tests 
    # also have these flags
    target_compile_options(fqzcomp28Core PUBLIC
        -Wall
        -Wextra
        -Wconversion
        -Wold-style-cast
        -Wcast-align
        -Wnrvo
        -Wpessimizing-move
        -frecord-gcc-switches  # to retain certain flags in the executable
        -g3
        $<$<CONFIG:DEBUG>:-O0>
    )
    target_link_options(fqzcomp28Core PUBLIC
        -frecord-gcc-switches
    )
endif()

add_executable(fqzcomp28 entry_point.cpp)
target_link_libraries(fqzcomp28 PRIVATE fqzcomp28Core)

# copy (move?) executable to 
add_custom_command(TARGET fqzcomp28 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fqzcomp28> PROJECT_SOURCE_DIR)
