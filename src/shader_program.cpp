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

VertexFragmentShaderProgram::VertexFragmentShaderProgram(Shader *vertexShader, Shader *fragmentShader)
{
    // Validate shader types
    if (vertexShader->type != ShaderType::VERTEX_SHADER)
    {
        logError("The given vertex shader is not a vertex shader!");
        return;
    }
    if (fragmentShader->type != ShaderType::FRAGMENT_SHADER)
    {
        logError("The given fragment shader is not a fragment shader!");
        return;
    }

    programID = glCreateProgram();
    // Attach shaders
    glAttachShader(programID, vertexShader->shaderID);
    glAttachShader(programID, fragmentShader->shaderID);
    // Link program
    glLinkProgram(programID);
    int linkResult;
    linkResult = validateShaderProgramLinking(programID);
    // Link success
    if (linkResult == 0)
    {
        isInitialised = true;
    }
}

VertexFragmentShaderProgram::~VertexFragmentShaderProgram()
{
    glDeleteProgram(programID);
    logTrace("Vertex/Fragment shader program deleted.");
}

void VertexFragmentShaderProgram::use()
{
    glUseProgram(programID);
}

ComputeShaderProgram::ComputeShaderProgram(Shader *computeShader)
{
    // Validate shader types
    if (computeShader->type != ShaderType::COMPUTE_SHADER)
    {
        logError("The given compute shader is not a compute shader!");
        return;
    }

    programID = glCreateProgram();
    // Attach shaders
    glAttachShader(programID, computeShader->shaderID);
    // Link program
    glLinkProgram(programID);
    int linkResult;
    linkResult = validateShaderProgramLinking(programID);
    // Link success
    if (linkResult == 0)
    {
        isInitialised = true;
    }
}

ComputeShaderProgram::~ComputeShaderProgram()
{
    glDeleteProgram(programID);
    logTrace("Compute shader program deleted.");
}

void ComputeShaderProgram::use()
{
    glUseProgram(programID);
}