#version 460 core

layout(location=0) out vec4 outColor;

void main()
{
    outColor = vec4(gl_FragCoord.x * 1.0f, gl_FragCoord.y * 0.2f, gl_FragCoord.z * 0.3f, 1.0f);
} 