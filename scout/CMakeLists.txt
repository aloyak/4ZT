cmake_minimum_required(VERSION 4.0)
project(scout LANGUAGES CXX)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS src/*.cpp)

include(cmake/sdl3.cmake)

add_executable(scout ${CLIENT_SOURCES})
target_include_directories(scout PRIVATE include)
target_link_libraries(scout PRIVATE FourZT::raytracer SDL3::SDL3)