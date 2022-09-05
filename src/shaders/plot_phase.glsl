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
    float maxValue = PI;

    float realValue = texture(inRealFieldTexture, inTextureCoordinate).r;
    float imagValue = texture(inImagFieldTexture, inTextureCoordinate).r;

    // Load the field value
	float fieldValue = atan(imagValue, realValue);

    // Clamp the field value to between +-maxValue
    fieldValue = clamp(fieldValue, -maxValue, +maxValue);
    // Rescale value to between 0 and 1
    fieldValue = (fieldValue + maxValue) / (2.0f * maxValue);
    fieldValue = clamp(fieldValue, 0.0f, 1.0f);
    
    // Sample color map using scaled field value
    outColor = texture(inColorMap, vec2(fieldValue, 0.5f));
}