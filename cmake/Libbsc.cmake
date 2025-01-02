include(FetchContent)

FetchContent_Declare(
    bsc
    GIT_REPOSITORY https://github.com/IlyaGrebnov/libbsc
    GIT_TAG        v3.3.4
)

FetchContent_MakeAvailable(bsc)

# Run the `find` command to locate .cpp files under a specific directory
set(BSC_SOURCE_PREFIX ${bsc_SOURCE_DIR}/libbsc)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E chdir ${BSC_SOURCE_PREFIX}
    find . -name "*.cpp" -or -name "*.c"
    OUTPUT_VARIABLE BSC_SOURCES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\n" ";" BSC_SOURCES "${BSC_SOURCES}")
list(TRANSFORM BSC_SOURCES PREPEND ${BSC_SOURCE_PREFIX}/)

add_library(bsc STATIC ${BSC_SOURCES})
