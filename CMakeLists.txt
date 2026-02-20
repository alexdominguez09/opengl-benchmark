cmake_minimum_required(VERSION 3.15)
project(opengl-benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Add GLAD source
add_library(glad STATIC glad/src/gl.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

# stb_image for texture loading
add_library(stb_image STATIC src/stb_image.cpp)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Main executable
add_executable(opengl-benchmark 
    src/main.cpp
    src/shader.cpp
    src/model_loader.cpp
    src/texture_loader.cpp
    src/fps_counter.cpp
    src/text_renderer.cpp
)

target_include_directories(opengl-benchmark PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)

target_link_libraries(opengl-benchmark 
    OpenGL::GL
    glfw
    glm::glm
    glad
    stb_image
)

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
