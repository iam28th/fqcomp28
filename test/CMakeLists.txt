include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG        v3.7.1
)
FetchContent_MakeAvailable(Catch2)


add_executable(unit_tests
    test_utils.cpp
    fastq_io_test.cpp
    headers_test.cpp
    encoding_context_test.cpp
)

target_link_libraries(unit_tests PRIVATE
                      fqzcomp28Core Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(unit_tests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
