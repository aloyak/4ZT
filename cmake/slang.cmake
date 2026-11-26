# Compiles raytracer/shaders/*.slang to SPIR-V and provides deploy_shaders(<target>)

find_program(SLANGC slangc
    HINTS "$ENV{HOME}/tools/slang/bin" "$ENV{VULKAN_SDK}/bin"
    REQUIRED)

# Anchored to this file's location, so it works from any subdirectory
set(FOURZT_SHADER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../raytracer/shaders)
cmake_path(NORMAL_PATH FOURZT_SHADER_SRC_DIR)
set(FOURZT_SHADER_BIN_DIR ${CMAKE_BINARY_DIR}/shaders)

# shaders/*.slang      -> entry-point shaders, one .spv each
# shaders/lib/*.slang  -> shared modules, only used via `import`
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS ${FOURZT_SHADER_SRC_DIR}/*.slang)
file(GLOB SHADER_LIBS    CONFIGURE_DEPENDS ${FOURZT_SHADER_SRC_DIR}/lib/*.slang)

if(NOT SHADER_SOURCES)
    message(FATAL_ERROR "No .slang files found in ${FOURZT_SHADER_SRC_DIR}")
endif()

set(FOURZT_SHADER_SPV "")
foreach(src IN LISTS SHADER_SOURCES)
    get_filename_component(name ${src} NAME_WE)
    set(spv ${FOURZT_SHADER_BIN_DIR}/${name}.spv)
    add_custom_command(
        OUTPUT  ${spv}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FOURZT_SHADER_BIN_DIR}
        COMMAND ${SLANGC} ${src} -target spirv
                -I ${FOURZT_SHADER_SRC_DIR}/lib
                -o ${spv}
        DEPENDS ${src} ${SHADER_LIBS}
        COMMENT "slangc ${name}.slang -> ${name}.spv"
        VERBATIM)
    list(APPEND FOURZT_SHADER_SPV ${spv})
endforeach()

add_custom_target(raytracer_shaders DEPENDS ${FOURZT_SHADER_SPV})

# Global property: visible from every directory, unlike a normal variable
# (this file is included from raytracer/, but deploy_shaders is called from
# sibling directories like standalone/ and scout/)
set_property(GLOBAL PROPERTY FOURZT_SHADER_SPV_FILES "${FOURZT_SHADER_SPV}")


function(deploy_shaders target)
    get_property(spv_files GLOBAL PROPERTY FOURZT_SHADER_SPV_FILES)
    if(NOT spv_files)
        message(FATAL_ERROR "deploy_shaders(${target}): no compiled shaders registered")
    endif()

    add_dependencies(${target} raytracer_shaders)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${spv_files} "$<TARGET_FILE_DIR:${target}>/shaders"
        VERBATIM)
endfunction()