#pragma once

// Internal libraries
#include "shader.h"

// Shader programs that contain vertex and fragment shader stages.
class VertexFragmentShaderProgram
{
public:
    // Signals that the shader program was successfully initialised if true.
    bool isInitialised = false;
    // OpenGL shader program ID
    uint32_t programID = 0;

    // Constructor
    VertexFragmentShaderProgram(Shader *vertexShader, Shader *fragmentShader);
    // Destructor
    ~VertexFragmentShaderProgram();

    // Use the vertex-fragment shader program
    void use();
};

// Compute shader program.
class ComputeShaderProgram
{
public:
    // Signals that the shader program was successfully initialised if true.
    bool isInitialised = false;
    // OpenGL shader program ID
    uint32_t programID = 0;

    // Constructor
    ComputeShaderProgram(Shader *computeShader);
    // Destructor
    ~ComputeShaderProgram();

    // Use the compute shader shader program
    void use();
};