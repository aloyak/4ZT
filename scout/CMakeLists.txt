cmake_minimum_required(VERSION 4.0)
project(scout LANGUAGES CXX)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS src/*.cpp)

find_package(Vulkan REQUIRED)

include(cmake/sdl3.cmake)
include(cmake/volk.cmake)
include(cmake/vma.cmake)
include(cmake/imgui.cmake)

add_executable(scout ${CLIENT_SOURCES})
target_include_directories(scout PRIVATE include)
target_link_libraries(scout PRIVATE FourZT::raytracer 
    SDL3::SDL3 # windowing and input
    Vulkan::Vulkan volk VulkanMemoryAllocator # vulkan
    imgui # gui, port to rmlui at some point
)

# upload the resources folder to the build directory, right next to the executable
add_custom_command(TARGET scout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:scout>/resources"
)
