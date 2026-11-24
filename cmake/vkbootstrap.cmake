include(FetchContent)

FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG        v1.4.368
)

FetchContent_MakeAvailable(vk-bootstrap)
