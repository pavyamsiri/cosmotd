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
        logError("Failed to link shader program! - %s", infoLog);
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

    logDebug("Creating vertex/fragment shader program...");
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
        logDebug("Vertex/fragment shader program created with ID %d", programID);
        isInitialised = true;
    }
}

VertexFragmentShaderProgram::~VertexFragmentShaderProgram()
{
    glDeleteProgram(programID);
    logTrace("Vertex/Fragment shader program deleted.");
}

const void VertexFragmentShaderProgram::use() const
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

    logDebug("Creating compute shader program...");
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
        logDebug("Compute shader program created with ID %d", programID);
        isInitialised = true;
    }
}

ComputeShaderProgram::~ComputeShaderProgram()
{
    glDeleteProgram(programID);
    logTrace("Compute shader program deleted.");
}

void const ComputeShaderProgram::use() const
{
    glUseProgram(programID);
}