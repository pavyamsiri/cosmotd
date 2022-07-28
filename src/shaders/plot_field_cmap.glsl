#version 460 core

// Vertex shader inputs
layout(location = 0) in vec2 inTextureCoordinate;

// Fragment shader outputs
layout (location = 0) out vec4 outColor;

// Uniforms
layout (binding = 0) uniform sampler2D inColorMap;
layout (binding = 1) uniform sampler2D inFieldTexture;
layout (location = 0) uniform float eta;

void main()
{
    float maxValue = min(1.1f * eta, eta + 0.1f);
	float fieldValue = texture(inFieldTexture, inTextureCoordinate).r;
    fieldValue = clamp(fieldValue, -maxValue, +maxValue);
    fieldValue = (fieldValue + maxValue) / (2.0f * maxValue);
    fieldValue = clamp(fieldValue, 0.0f, 1.0f);
    outColor = texture(inColorMap, vec2(fieldValue, 0.5f));
}