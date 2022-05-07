# Minimum required version of Cmake needed to run the build script
cmake_minimum_required (VERSION 3.9.0)
# Name of the project
project(cosmotd)

# Turn extra GLFW build docs, tests and examples off 
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Add the glfw subdirectory. Subdirectories have their own CMakeLists.txt.
add_subdirectory(external/glfw-3.3.7)

# Create an executable with main.cpp as its entry point. glad.c is also to be added as a source file.
add_executable(cosmotd 
    src/main.cpp
    external/glad/src/glad.c
)
# Add the path to the glad `include` directory to the target
target_include_directories(cosmotd PRIVATE external/glad/include) # Not entirely sure what PRIVATE means

# Specify the libraries for the linker to the target
target_link_libraries(cosmotd PRIVATE glfw) # Not entirely sure what PRIVATE means
