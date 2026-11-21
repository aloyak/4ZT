include(FetchContent)

set(VMA_BUILD_SAMPLE        OFF CACHE INTERNAL "")
set(VMA_BUILD_DOCUMENTATION OFF CACHE INTERNAL "")

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)