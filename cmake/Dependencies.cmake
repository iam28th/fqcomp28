include (FetchContent)

# TODO: use installed dependancies where possible
FetchContent_Declare(
    cli11_proj
    GIT_REPOSITORY "https://github.com/CLIUtils/CLI11.git"
    GIT_TAG "v2.4.2"
    QUIET
)

# TODO: maybe only static is sufficient
set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_COMPRESSION ON)
set(ZSTD_BUILD_DECOMPRESSION ON)

set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_DICTBUILDER OFF)
set(ZSTD_BUILD_DEPRECATED OFF)
set(ZSTD_BUILD_TESTS OFF)
set(ZSTD_BUILD_PROGRAMS OFF)

FetchContent_Declare(
    zstd
    GIT_REPOSITORY "https://github.com/iam28th/zstd"
    GIT_TAG "b010526d"
    SOURCE_SUBDIR build/cmake
    QUIET
)

FetchContent_MakeAvailable(cli11_proj zstd)
