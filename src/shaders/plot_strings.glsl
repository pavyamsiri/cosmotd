#version 460 core

// Vertex shader inputs
layout(location = 0) in vec2 inTextureCoordinate;

// Fragment shader outputs
layout (location = 0) out vec4 outColor;

// Uniforms
layout (binding = 0) uniform sampler2D inFieldColorMap;
layout (binding = 1) uniform sampler2D inStringColorMap;
layout (binding = 2) uniform sampler2D inRealFieldTexture;
layout (binding = 3) uniform sampler2D inImagFieldTexture;
layout (binding = 4) uniform sampler2D inStringTexture;

const float PI = 3.1415926535897932384626433832795f;

float convertValueToTextureUV(float value, float maxValue) {
    // Clamp the field value to between +-maxValue
    float returnValue = clamp(value, -maxValue, +maxValue);
    // Rescale value to between 0 and 1
    returnValue = (returnValue + maxValue) / (2.0f * maxValue);
    returnValue = clamp(returnValue, 0.0f, 1.0f);
    return returnValue;
}

void main()
{
    float maxValue = PI;

    float realValue = texture(inRealFieldTexture, inTextureCoordinate).r;
    float imagValue = texture(inImagFieldTexture, inTextureCoordinate).r;
    float stringValue = texture(inStringTexture, inTextureCoordinate).r;

    // Load the field value
	float fieldValue = atan(imagValue, realValue);

    float fieldUV = convertValueToTextureUV(fieldValue, PI);
    float stringUV = convertValueToTextureUV(stringValue, 1);
    
    // Sample color map using scaled field value
    vec4 fieldColor = texture(inFieldColorMap, vec2(fieldUV, 0.5f));
    vec4 stringColor = texture(inStringColorMap, vec2(stringUV, 0.5f));

    outColor = mix(fieldColor, stringColor, 0.75f * abs(stringValue));
}