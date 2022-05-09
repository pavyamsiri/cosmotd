#version 460 core

in vec2 vTexCoord;

out vec4 fragColor;

// texture samplers
uniform sampler2D displayTexture;

void main()
{
    float maxValue = 1.1f;
	float fieldValue = (texture(displayTexture, vTexCoord).r + maxValue) / 2;
    fragColor = vec4(fieldValue, fieldValue, fieldValue, 1.0f);
}