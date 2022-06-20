#include <shader_program.h>
#include <sstream>
#include <log.h>
#include <glad/glad.h>

int validateShaderProgramLinking(uint32_t programID)
{
    GLint success;
    GLchar infoLog[1024];

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 1024, NULL, infoLog);
        std::stringstream errorStream;
        errorStream << "Failed to link shader program! - " << infoLog;
        logError(errorStream.str().c_str());
        return -1;
    }
    return 0;
}

int ShaderProgram::linkVertexFragmentProgram(Shader vertexShader, Shader fragmentShader, uint32_t *programID)
{
    // Validate shader types
    if (vertexShader.type != ShaderType::VERTEX_SHADER)
    {
        logError("The given vertex shader is not a vertex shader!");
        return -1;
    }
    if (fragmentShader.type != ShaderType::FRAGMENT_SHADER)
    {
        logError("The given fragment shader is not a fragment shader!");
        return -1;
    }

    *programID = glCreateProgram();
    // Attach shaders
    glAttachShader(*programID, vertexShader.shaderID);
    glAttachShader(*programID, fragmentShader.shaderID);
    // Link program
    glLinkProgram(*programID);
    int linkResult;
    linkResult = validateShaderProgramLinking(*programID);
    if (linkResult != 0)
    {
        return linkResult;
    }
    return 0;
}

int ShaderProgram::linkComputeProgram(Shader computeShader, uint32_t *programID)
{
    // Validate shader types
    if (computeShader.type != ShaderType::COMPUTE_SHADER)
    {
        logError("The given compute shader is not a compute shader!");
        return -1;
    }

    *programID = glCreateProgram();
    // Attach shaders
    glAttachShader(*programID, computeShader.shaderID);
    // Link program
    glLinkProgram(*programID);
    int linkResult;
    linkResult = validateShaderProgramLinking(*programID);
    if (linkResult != 0)
    {
        return linkResult;
    }

    return 0;
}