cmake_minimum_required(VERSION 4.0)
project(client LANGUAGES CXX)

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE include)
target_link_libraries(client PRIVATE FourZT::raytracer)