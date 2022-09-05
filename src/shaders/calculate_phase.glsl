#version 460 core
// Work groups
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
// In: Real field texture
layout(rgba32f, binding = 0) readonly uniform image2D inRealFieldTexture;
// In: Imaginary field texture
layout(rgba32f, binding = 1) readonly uniform image2D inImagFieldTexture;
// Out: Phase texture
layout(r32f, binding = 2) writeonly uniform image2D outPhaseTexture;

void main()
{
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(inRealFieldTexture);
    vec4 realValue = imageLoad(inRealFieldTexture, pos);
    vec4 imagValue = imageLoad(inImagFieldTexture, pos);
    // Calculate phase
    vec4 phaseAngle = atan(imagValue, realValue);
    
    // Store phase
    imageStore(outPhaseTexture, pos, vec4(phaseAngle.r, 0.0f, 0.0f, 0.0f));
}