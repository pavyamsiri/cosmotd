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
        logError("Failed to compile shader! - %s", infoLog);
        return -1;
    }
    else
    {
        return 0;
    }
}

Shader::Shader(const char *shaderPath, ShaderType type) : type(type)
{
    logTrace("Compiling shader from file...");
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
        logError("Failed to read %s from file! - ", convertShaderTypeToString(type), e.what());
        return;
    }

    const char *csShaderCode = shaderCode.c_str();

    int glShaderType;
    try
    {
        glShaderType = convertShaderTypeToGLShaderType(type).value();
    }
    catch (const std::bad_optional_access &e)
    {
        logError("Invalid shader type!");
        return;
    }

    shaderID = glCreateShader(glShaderType);

    glShaderSource(shaderID, 1, &csShaderCode, NULL);
    glCompileShader(shaderID);

    // Check compilation error
    int compilationResult = validateShaderCompilation(shaderID);
    if (compilationResult != 0)
    {
        glDeleteShader(shaderID);
        return;
    }

    // Initialisation complete
    this->isInitialised = true;

    logTrace("Shader initialised successfully!");
    return;
}

Shader::~Shader()
{
    glDeleteShader(this->shaderID);
    logTrace("Shader deleted.");
}
