cmake_minimum_required(VERSION 4.0)
project(raytracer LANGUAGES CXX)

include(cmake/stb_image_write.cmake)

file(GLOB_RECURSE RAYTRACER_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(raytracer STATIC ${RAYTRACER_SOURCES})
add_library(FourZT::raytracer ALIAS raytracer)

target_include_directories(raytracer PUBLIC include)
target_compile_features(raytracer PUBLIC cxx_std_20)
target_link_libraries(raytracer PRIVATE stb_image_write)