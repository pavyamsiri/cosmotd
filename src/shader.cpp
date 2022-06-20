#include <string>
#include <fstream>
#include <sstream>
#include <optional>
#include <shader.h>
#include <log.h>
#include <glad/glad.h>

std::optional<int> convertShaderTypeToGLShaderType(ShaderType type)
{
    switch (type)
    {
    case ShaderType::VERTEX_SHADER:
        return std::make_optional(GL_VERTEX_SHADER);
    case ShaderType::FRAGMENT_SHADER:
        return std::make_optional(GL_FRAGMENT_SHADER);
    case ShaderType::COMPUTE_SHADER:
        return std::make_optional(GL_COMPUTE_SHADER);
    default:
        return std::nullopt;
    }
}

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
    default:
        return "UNKNOWN_SHADER";
    }
}

int validateShaderCompilation(uint32_t shaderID)
{
    GLint success;
    GLchar infoLog[1024];

    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
        std::stringstream errorStream;
        errorStream << "Failed to compile shader! - " << infoLog;
        logError(errorStream.str().c_str());
        return -1;
    }
    else
    {
        return 0;
    }
}

int Shader::compileShaderFromFile(const char *shaderPath, ShaderType type, Shader *shader)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shaderFile.open(shaderPath);
        std::stringstream shaderStringStream;

        shaderStringStream << shaderFile.rdbuf();

        shaderFile.close();

        shaderCode = shaderStringStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::stringstream errorStream;
        errorStream << "Failed to read " << convertShaderTypeToString(type) << " from file! - " << e.what();
        logError(errorStream.str().c_str());
        return -1;
    }

    const char *csShaderCode = shaderCode.c_str();

    uint32_t shaderID;

    int glShaderType;
    try
    {
        glShaderType = convertShaderTypeToGLShaderType(type).value();
    }
    catch (const std::bad_optional_access &e)
    {
        logError("Invalid shader type!");
        return -1;
    }

    shaderID = glCreateShader(glShaderType);

    glShaderSource(shaderID, 1, &csShaderCode, NULL);
    glCompileShader(shaderID);

    // Check compilation error
    int compilationResult = validateShaderCompilation(shaderID);
    if (compilationResult != 0)
    {
        glDeleteShader(shaderID);
        return compilationResult;
    }

    // Link shader to the compiled shader ID
    shader->shaderID = shaderID;
    shader->type = type;
    return 0;
}

void Shader::deleteShader()
{
    glDeleteShader(shaderID);
}