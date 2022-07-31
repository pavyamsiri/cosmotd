#version 460 core

// Vertex buffer inputs
layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTextureCoordinate;

// Vertex shader outputs
layout (location = 0) out vec2 inTextureCoordinate;

void main()
{
    inTextureCoordinate = aTextureCoordinate;
    gl_Position = vec4(aPosition, 0.0f, 1.0f);
}