#version 460 core
// Work groups
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
// In: Real field texture
layout(rgba32f, binding = 0) restrict readonly uniform image2D inRealFieldTexture;
// In: Imaginary field texture
layout(rgba32f, binding = 1) restrict readonly uniform image2D inImagFieldTexture;
// Out: Phase texture
layout(r32f, binding = 2) restrict writeonly uniform image2D outPhaseTexture;

const float PI = 3.1415926535897932384626433832795f;

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(inRealFieldTexture);
    float realValue = imageLoad(inRealFieldTexture, pos).r;
    float imagValue = imageLoad(inImagFieldTexture, pos).r;
    // Calculate phase
    float phaseAngle = atan(imagValue, realValue);
    phaseAngle = clamp(phaseAngle, -PI, +PI);

    // Store phase
    imageStore(outPhaseTexture, pos, vec4(phaseAngle, 0.0f, 0.0f, 0.0f));
}