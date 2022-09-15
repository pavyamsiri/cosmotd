#pragma once
#include <stdint.h>

// Types of shaders
enum class ShaderType
{
    VERTEX_SHADER = 0,
    FRAGMENT_SHADER,
    COMPUTE_SHADER,
    UNKNOWN_SHADER,
};

// Wraps an OpenGL shader
class Shader
{
public:
    // Signals that the shader was successfully initialised if true.
    bool isInitialised = false;
    // OpenGL shader ID
    uint32_t shaderID = 0;
    // Shader type
    ShaderType type = ShaderType::UNKNOWN_SHADER;

    // Constructor
    Shader(const char *shaderPath, ShaderType type);
    // Destructor
    ~Shader();

    // Loads a shader from a file and compiles it.
    static int compileShaderFromFile(const char *shaderPath, ShaderType type, Shader *shader);
};