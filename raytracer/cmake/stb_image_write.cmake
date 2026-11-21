include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(stb)

# stb is header-only, so expose it as an interface library
add_library(stb_image_write INTERFACE)
target_include_directories(stb_image_write INTERFACE ${stb_SOURCE_DIR})