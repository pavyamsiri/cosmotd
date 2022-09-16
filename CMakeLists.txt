# Minimum required version of Cmake needed to run the build script
cmake_minimum_required(VERSION 3.9.0)

# Name of the project
project(cosmotd)

# C++ version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Log levels
# add_compile_definitions(LOG_LEVEL_LOOP)
add_compile_definitions(LOG_LEVEL_TRACE)
add_compile_definitions(LOG_LEVEL_DEBUG)
add_compile_definitions(LOG_LEVEL_INFO)
add_compile_definitions(LOG_LEVEL_WARNING)
add_compile_definitions(LOG_LEVEL_ERROR)
add_compile_definitions(LOG_LEVEL_FATAL)

# Turn extra GLFW build docs, tests and examples off
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Add the glfw subdirectory. Subdirectories have their own CMakeLists.txt.
add_subdirectory(external/glfw)
add_subdirectory(external/nativefiledialog-extended)

# Create an executable with main.cpp as its entry point. glad.c is also to be added as a source file.
add_executable(cosmotd
    src/main.cpp
    src/log.cpp
    src/application.cpp
    src/shader.cpp
    src/shader_program.cpp
    src/buffer.cpp
    src/framebuffer.cpp
    src/texture.cpp
    src/simulation.cpp
    external/glad/src/glad.c
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_impl_glfw.cpp
    external/imgui/imgui_impl_opengl3.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui.cpp
)

# Add the path to the glad `include` directory to the target
target_include_directories(cosmotd PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    external/glad/include
    external/glfw/include
    external/imgui
    external/stbi
) # Not entirely sure what PRIVATE means

# Specify the libraries for the linker to the target
target_link_libraries(cosmotd PRIVATE glfw) # Not entirely sure what PRIVATE means
target_link_libraries(cosmotd PRIVATE nfd)

# Copy over shaders folder
add_custom_target(copy_shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
)
add_dependencies(cosmotd copy_shaders)

# Copy over data folder
add_custom_target(copy_data
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/data ${CMAKE_CURRENT_BINARY_DIR}/data
)
add_dependencies(cosmotd copy_data)

# Copy over colormap folder
add_custom_target(copy_colormap
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/src/colormaps ${CMAKE_CURRENT_BINARY_DIR}/colormaps
)
add_dependencies(cosmotd copy_colormap)