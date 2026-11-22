include(FetchContent)

# cmake 4.0 would reject the policy version 3.5
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(TINYEXR_BUILD_SAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    tinyexr
    GIT_REPOSITORY https://github.com/syoyo/tinyexr.git
    GIT_TAG        release
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(tinyexr)

target_include_directories(tinyexr PUBLIC ${tinyexr_SOURCE_DIR})
set_target_properties(miniz PROPERTIES POSITION_INDEPENDENT_CODE ON)