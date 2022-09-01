#pragma once
#include "shader.h"

class VertexFragmentShaderProgram
{
public:
    // Attributes
    uint32_t programID;
    bool isInitialised = false;

    // Constructor and destructor
    VertexFragmentShaderProgram(Shader *vertexShader, Shader *fragmentShader);
    ~VertexFragmentShaderProgram();

    // Use the vertex-fragment shader program
    void const use() const;
};

class ComputeShaderProgram
{
public:
    // Attributes
    uint32_t programID;
    bool isInitialised = false;

    // Constructor and destructor
    ComputeShaderProgram(Shader *computeShader);
    ~ComputeShaderProgram();

    // Use the compute shader shader program
    void const use() const;
};