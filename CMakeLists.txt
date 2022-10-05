cmake_minimum_required(VERSION 3.10)

# Set the project name and version
set(TARGET_NAME nameless)
project(${TARGET_NAME} VERSION 1.0)

# C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Dependencies
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_subdirectory(lib/assimp)

# Add files
file(GLOB SOURCES src/*.cpp lib/glad/src/glad.c)

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
    "${PROJECT_SOURCE_DIR}/lib/stb_image/")

# C++ warnings and compiler flags
if(MSVC)
    target_compile_options(${TARGET_NAME} PUBLIC /W4 /Wv:18)
else()
    target_compile_options(${TARGET_NAME} PRIVATE -O3 -Wall -Wextra -pedantic -fdiagnostics-color=always)
endif()

# Dependencies
target_link_libraries(${TARGET_NAME} glfw glm assimp)