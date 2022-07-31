#pragma once
#include <stdint.h>

enum class ShaderType
{
    VERTEX_SHADER = 0,
    FRAGMENT_SHADER,
    COMPUTE_SHADER,
    GEOMETRY_SHADER,
    UNKNOWN_SHADER,
};

class Shader
{
public:
    uint32_t shaderID;
    ShaderType type = ShaderType::UNKNOWN_SHADER;
    bool isInitialised = false;

    Shader(const char *shaderPath, ShaderType type);
    ~Shader();

    static int compileShaderFromFile(const char *shaderPath, ShaderType type, Shader *shader);
    void deleteShader();
};