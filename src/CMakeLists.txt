include(${CMAKE_SOURCE_DIR}/cmake/Dependencies.cmake)

add_library(fqcomp28Core OBJECT
    app.cpp
    archive.cpp
    fastq_io.cpp
    fse_quality.cpp
    fse_sequence.cpp
    headers.cpp
    memcompress.cpp
    prepare.cpp
    process.cpp
    report.cpp
    utils.cpp
    workspace.cpp
)

target_include_directories(fqcomp28Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(fqcomp28Core PRIVATE
    CLI11::CLI11
    libzstd_static
    bsc
)

# mark include folders as 'system', so that the headers don't yield warnings
target_include_directories(fqcomp28Core SYSTEM PUBLIC
    $<TARGET_PROPERTY:libzstd_static,INTERFACE_INCLUDE_DIRECTORIES>
    ${bsc_SOURCE_DIR}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # public so that fqcomp28 executable and unit_tests
    # also have these flags
    target_compile_options(fqcomp28Core PUBLIC
        -Werror
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
    target_link_options(fqcomp28Core PUBLIC
        -frecord-gcc-switches
    )
endif()

add_executable(fqcomp28 entry_point.cpp)
target_link_libraries(fqcomp28 PRIVATE fqcomp28Core)

# move executable to project root
add_custom_command(TARGET fqcomp28
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:fqcomp28> ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:fqcomp28>)
