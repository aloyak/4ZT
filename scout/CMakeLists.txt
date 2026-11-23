cmake_minimum_required(VERSION 4.0)
project(scout LANGUAGES CXX)

file(GLOB_RECURSE SCOUT_SOURCES CONFIGURE_DEPENDS src/*.cpp)

# volk, VMA and Vulkan come in through FourZT::raytracer
include(sdl3)
include(imgui)

add_executable(scout ${SCOUT_SOURCES})
target_include_directories(scout PRIVATE include)
target_link_libraries(scout PRIVATE
    FourZT::raytracer
    SDL3::SDL3 # windowing and input
    imgui      # gui, port to rmlui at some point
)

# copy the resources folder next to the executable
add_custom_command(TARGET scout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:scout>/resources"
)