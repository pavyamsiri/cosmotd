// Standard libraries
#include <string>
#include <fstream>
#include <sstream>

// External libraries
#include <glad/glad.h>

// Internal libraries
#include "log.h"
#include "shader.h"

// Helper function that converts a ShaderType to its corresponding GLenum
GLenum convertShaderTypeToGLShaderType(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX_SHADER:
        return GL_VERTEX_SHADER;
    case ShaderType::FRAGMENT_SHADER:
        return GL_FRAGMENT_SHADER;
    case ShaderType::COMPUTE_SHADER:
        return GL_COMPUTE_SHADER;
    case ShaderType::UNKNOWN_SHADER:
    default:
        logError("Invalid shader type %d!", type);
        return 0;
    }
}

// Helper function that returns a shader type's name as a string
const char *convertShaderTypeToString(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX_SHADER:
        return "VERTEX_SHADER";
    case ShaderType::FRAGMENT_SHADER:
        return "FRAGMENT_SHADER";
    case ShaderType::COMPUTE_SHADER:
        return "COMPUTE_SHADER";
    case ShaderType::UNKNOWN_SHADER:
        logError("Invalid shader type!");
        return "UNKNOWN_SHADER";
    }
}

// Helper function that checks shader compilation, returning true if compilation is successful.
bool validateShaderCompilation(uint32_t shaderID)
{
    GLint success;
    GLchar infoLog[1024];

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
        logError("Failed to compile shader with ID %d! - %s", shaderID, infoLog);
        return false;
    }
    else
    {
        return true;
    }
}

Shader::Shader(const char *shaderPath, ShaderType type) : type(type)
{
    logDebug("Shader is being loaded from file located at %s", shaderPath);
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        // Open shader file
        shaderFile.open(shaderPath);

        // Stream code into buffer and turn into string
        std::stringstream shaderStringStream;
        shaderStringStream << shaderFile.rdbuf();
        shaderCode = shaderStringStream.str();

        // Close shader file
        shaderFile.close();

        logTrace("Shader code successfully loaded from file located at %s", shaderPath);
    }
    // IO error
    catch (std::ifstream::failure &e)
    {
        logError(
            "Failed to read shader code of type %s from file located at %s. Exception: %s",
            convertShaderTypeToString(type), shaderPath, e.what());
        return;
    }

    // Convert to c string
    const char *csShaderCode = shaderCode.c_str();

    // Generate OpenGL shader
    GLenum glShaderType = convertShaderTypeToGLShaderType(type);
    shaderID = glCreateShader(glShaderType);

    // Attach source and compile
    glShaderSource(shaderID, 1, &csShaderCode, NULL);
    glCompileShader(shaderID);

    // Check for compilation errors
    bool compilationResult = validateShaderCompilation(shaderID);
    if (!compilationResult)
    {
        glDeleteShader(shaderID);
        logError("Shader with ID %d has been deleted due to a compilation error.", shaderID);
        return;
    }

    // Initialisation complete
    isInitialised = true;

    logDebug("Shader successfully created with ID %d.", shaderID);
    return;
}

Shader::~Shader()
{
    logDebug("Shader with ID %d is being destroyed...", shaderID);
    glDeleteShader(shaderID);
    logDebug("Shader with ID %d has been destroyed.", shaderID);
}
