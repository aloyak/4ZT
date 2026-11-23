cmake_minimum_required(VERSION 4.0)
project(standalone LANGUAGES CXX)

file(GLOB_RECURSE STANDALONE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(standalone ${STANDALONE_SOURCES})
target_include_directories(standalone PRIVATE include)
target_link_libraries(standalone PRIVATE FourZT::raytracer)