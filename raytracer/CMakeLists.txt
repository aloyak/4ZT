cmake_minimum_required(VERSION 4.0)
project(raytracer LANGUAGES CXX)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat libavcodec libavutil libswscale)

include(stb_image_write)
include(tinyexr)
include(volk)
include(vma)
include(vkbootstrap)

file(GLOB_RECURSE RAYTRACER_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(raytracer STATIC ${RAYTRACER_SOURCES})
add_library(FourZT::raytracer ALIAS raytracer)

target_include_directories(raytracer PUBLIC include)
target_compile_features(raytracer PUBLIC cxx_std_20)
target_link_libraries(raytracer
    PUBLIC  Vulkan::Headers volk VulkanMemoryAllocator vk-bootstrap::vk-bootstrap
    PRIVATE stb_image_write tinyexr PkgConfig::FFMPEG)