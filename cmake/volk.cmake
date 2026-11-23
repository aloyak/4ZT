include(FetchContent)

set(VOLK_INSTALL OFF CACHE INTERNAL "")

FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        1.4.350
)

FetchContent_MakeAvailable(volk)