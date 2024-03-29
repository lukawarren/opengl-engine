cmake_minimum_required(VERSION 3.10)

# Set the project name and version
set(TARGET_NAME glengine)
project(${TARGET_NAME} VERSION 1.0)

# C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Linking goes way faster for me under lld
find_program(LLD "lld")
if(LLD)
    message(STATUS "Using lld (faster than ld)")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
endif()

# Reduce Assimp binary size and build times
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_SMD_IMPORTER ON)
set(ASSIMP_BUILD_SIB_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

# Dependencies
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_subdirectory(lib/assimp)

# Add files
file(GLOB SOURCES src/*.cpp src/*/*.cpp
    lib/glad/src/glad.c
    lib/imgui/*.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
)

# For Visual Studio, add header and shader files
if(MSVC)
    file(GLOB HEADERS include/*.h)
    file(GLOB SHADERS res/*.vert res/*.frag res/*.geom)
    source_group("Headers" FILES ${HEADERS})
    source_group("Shaders" FILES ${SHADERS})
endif()

# Add project
add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_include_directories(${TARGET_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/lib/glfw/include"
    "${PROJECT_SOURCE_DIR}/lib/glm/include"
    "${PROJECT_SOURCE_DIR}/lib/glad/include"
    "${PROJECT_SOURCE_DIR}/lib/stb_image/"
    "${PROJECT_SOURCE_DIR}/lib/imgui/"
    "${PROJECT_SOURCE_DIR}/lib/imgui/backends"
)

# C++ warnings and compiler flags
if(MSVC)
    target_compile_options(${TARGET_NAME} PUBLIC /W4 /Wv:18)
else()
    target_compile_options(${TARGET_NAME} PRIVATE -O3 -g -Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

# Dependencies
target_link_libraries(${TARGET_NAME} glfw glm assimp)