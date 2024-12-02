include (FetchContent)

FetchContent_Declare (
    seqan3_fetch_content
    GIT_REPOSITORY "https://github.com/seqan/seqan3.git"
    GIT_TAG "3.4.0-rc.1"
    QUIET
)

FetchContent_Declare (
    cli11_proj
    GIT_REPOSITORY "https://github.com/CLIUtils/CLI11.git"
    GIT_TAG "v2.4.2"
    QUIET
)

FetchContent_MakeAvailable (seqan3_fetch_content cli11_proj)
