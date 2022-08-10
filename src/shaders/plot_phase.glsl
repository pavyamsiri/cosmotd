#version 460 core

// Vertex shader inputs
layout(location = 0) in vec2 inTextureCoordinate;

// Fragment shader outputs
layout (location = 0) out vec4 outColor;

// Uniforms
layout (binding = 0) uniform sampler2D inColorMap;
layout (binding = 1) uniform sampler2D inRealFieldTexture;
layout (binding = 2) uniform sampler2D inImagFieldTexture;

const float PI = 3.1415926535897932384626433832795f;

void main()
{
	float realValue = texture(inRealFieldTexture, inTextureCoordinate).r;
	float imagValue = texture(inImagFieldTexture, inTextureCoordinate).r;

    float phaseAngle = atan(imagValue, realValue);

    float maxValue = PI;
    float interpolationValue = clamp(phaseAngle, -maxValue, +maxValue);
    interpolationValue = (interpolationValue + maxValue) / (2.0f * maxValue);
    interpolationValue = clamp(interpolationValue, 0.0f, 1.0f);
    outColor = texture(inColorMap, vec2(interpolationValue, 0.5f));
}