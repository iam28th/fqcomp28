include (FetchContent)

FetchContent_Declare (
    seqan3_fetch_content
    GIT_REPOSITORY "https://github.com/seqan/seqan3.git"
    GIT_TAG "3.4.0-rc.1"
)

FetchContent_MakeAvailable(seqan3_fetch_content)
