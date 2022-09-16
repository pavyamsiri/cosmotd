// Standard libraries
#include <sstream>

// External libraries
#include <glad/glad.h>

// Internal libraries
#include "log.h"
#include "shader_program.h"

// Helper function that checks shader program linking, returning true if linking is successful.
bool validateShaderProgramLinking(uint32_t programID)
{
    GLint success;
    GLchar infoLog[1024];

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 1024, NULL, infoLog);
        logError("Failed to link shader program with ID %d! - %s", programID, infoLog);
        return false;
    }
    return true;
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
    logDebug("Vertex/fragment shader program is being created...");

    // Generate ID
    programID = glCreateProgram();
    // Attach shaders
    glAttachShader(programID, vertexShader->shaderID);
    glAttachShader(programID, fragmentShader->shaderID);
    // Link program
    glLinkProgram(programID);
    bool linkResult = validateShaderProgramLinking(programID);
    // Link success
    if (!linkResult)
    {
        glDeleteProgram(programID);
        logError("Vertex/fragment shader program with ID %d has been deleted due to a linking error.", programID);
        return;
    }
    else
    {
        isInitialised = true;
        logDebug("Vertex/fragment shader program has been created with ID %d.", programID);
        return;
    }
}

VertexFragmentShaderProgram::~VertexFragmentShaderProgram()
{
    logDebug("Vertex/fragment shader program with ID %d is being destroyed...", programID);
    glDeleteProgram(programID);
    logDebug("Vertex/fragment shader program with ID %d has been destroyed.", programID);
}

void VertexFragmentShaderProgram::use()
{
    logLoop("Using vertex/fragment shader program with ID %d.", programID);
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
    logDebug("Compute shader program is being created...");

    // Generate ID
    programID = glCreateProgram();
    // Attach shaders
    glAttachShader(programID, computeShader->shaderID);
    // Link program
    glLinkProgram(programID);
    bool linkResult = validateShaderProgramLinking(programID);
    // Link success
    if (!linkResult)
    {
        glDeleteProgram(programID);
        logError("Compute shader program with ID %d has been deleted due to a linking error.", programID);
        return;
    }
    else
    {
        isInitialised = true;
        logDebug("Compute shader program has been created with ID %d.", programID);
        return;
    }
}

ComputeShaderProgram::~ComputeShaderProgram()
{
    logDebug("Compute shader program with ID %d is being destroyed...", programID);
    glDeleteProgram(programID);
    logDebug("Compute shader program with ID %d has been destroyed.", programID);
}

void ComputeShaderProgram::use()
{
    logLoop("Using compute shader program with ID %d.", programID);
    glUseProgram(programID);
}