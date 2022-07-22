#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) readonly uniform image2D inFieldTexture;
layout(rgba32f, binding = 1) writeonly uniform image2D outLaplacianTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;


void main() {
    // Current cell position
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Need size to ensure periodic boundaries
    ivec2 size = imageSize(inFieldTexture);
    
    // Field value at current cell position
    vec4 current = imageLoad(inFieldTexture, pos);
    // One step
    vec4 leftOne = imageLoad(inFieldTexture, ivec2(mod(pos.x - 1, size.x), pos.y));
    vec4 rightOne = imageLoad(inFieldTexture, ivec2(mod(pos.x + 1, size.x), pos.y));
    vec4 downOne = imageLoad(inFieldTexture, ivec2(pos.x, mod(pos.y - 1, size.y)));
    vec4 upOne = imageLoad(inFieldTexture, ivec2(pos.x, mod(pos.y + 1, size.y)));
    // Two steps
    vec4 leftTwo = imageLoad(inFieldTexture, ivec2(mod(pos.x - 2, size.x), pos.y));
    vec4 rightTwo = imageLoad(inFieldTexture, ivec2(mod(pos.x + 2, size.x), pos.y));
    vec4 downTwo = imageLoad(inFieldTexture, ivec2(pos.x, mod(pos.y - 2, size.y)));
    vec4 upTwo = imageLoad(inFieldTexture, ivec2(pos.x, mod(pos.y + 2, size.y)));

    vec4 laplacian = -60.0f * current;
    laplacian += 16.0f * (leftOne + rightOne + downOne + upOne);
    laplacian -= (leftTwo + rightTwo + downTwo + upTwo);
    laplacian /= 12 * pow(dx, 2);
    // vec4 laplacian = 1/(12 * dx * dx) * (-60 * current + 16 * (leftOne + rightOne + downOne + upOne) - 1 * (leftTwo + rightTwo + downTwo + upTwo));

    imageStore(outLaplacianTexture, pos, laplacian);
}