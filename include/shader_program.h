#pragma once
#include <shader.h>

namespace ShaderProgram
{

    int linkVertexFragmentProgram(Shader vertexShader, Shader fragmentShader, uint32_t *programID);
    int linkComputeProgram(Shader computeShader, uint32_t *programID);
}
