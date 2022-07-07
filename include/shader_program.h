#pragma once
#include "shader.h"

class VertexFragmentShaderProgram
{
public:
    uint32_t programID;
    bool isInitialised = false;

    VertexFragmentShaderProgram(Shader *vertexShader, Shader *fragmentShader);
    ~VertexFragmentShaderProgram();

    void use();
};

class ComputeShaderProgram
{
public:
    uint32_t programID;
    bool isInitialised = false;

    ComputeShaderProgram(Shader *computeShader);
    ~ComputeShaderProgram();

    void const use() const;
};