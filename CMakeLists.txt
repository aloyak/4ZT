cmake_minimum_required(VERSION 4.0)
project(FourZT LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Vulkan REQUIRED)

add_subdirectory(raytracer)
add_subdirectory(standalone)
add_subdirectory(scout)