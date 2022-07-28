#version 460 core

// Vertex shader inputs
layout(location = 0) in vec2 inTextureCoordinate;

// Fragment shader outputs
layout (location = 0) out vec4 outColor;

// Uniforms
layout (binding = 0) uniform sampler2D inFieldTexture;

void main()
{
    outColor = texture(inFieldTexture, inTextureCoordinate);
}