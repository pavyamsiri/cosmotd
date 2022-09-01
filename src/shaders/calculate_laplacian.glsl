#version 460 core
// Work group specification
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
// In: Field texture
layout(rgba32f, binding = 0) readonly uniform image2D inFieldTexture;
// Out: Laplacian texture
layout(r32f, binding = 1) writeonly uniform image2D outLaplacianTexture;

// Uniforms: spatial interval
layout(location=0) uniform float dx;


void main() {
    // Current cell position
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Need size to ensure periodic boundaries
    ivec2 size = imageSize(inFieldTexture);

    // Horizontal
    ivec2 leftOnePos = ivec2(mod(pos.x - 1, size.x), pos.y);
    ivec2 rightOnePos = ivec2(mod(pos.x + 1, size.x), pos.y);
    ivec2 leftTwoPos = ivec2(mod(pos.x - 2, size.x), pos.y);
    ivec2 rightTwoPos = ivec2(mod(pos.x + 2, size.x), pos.y);
    // Vertical
    ivec2 downOnePos = ivec2(pos.x, mod(pos.y - 1, size.y));
    ivec2 upOnePos = ivec2(pos.x, mod(pos.y + 1, size.y));
    ivec2 downTwoPos = ivec2(pos.x, mod(pos.y - 2, size.y));
    ivec2 upTwoPos = ivec2(pos.x, mod(pos.y + 2, size.y));
    
    // Field value at current cell position
    vec4 current = imageLoad(inFieldTexture, pos);
    // One step
    vec4 leftOne = imageLoad(inFieldTexture, leftOnePos);
    vec4 rightOne = imageLoad(inFieldTexture, rightOnePos);
    vec4 downOne = imageLoad(inFieldTexture, downOnePos);
    vec4 upOne = imageLoad(inFieldTexture, upOnePos);
    // Two steps
    vec4 leftTwo = imageLoad(inFieldTexture, leftTwoPos);
    vec4 rightTwo = imageLoad(inFieldTexture, rightTwoPos);
    vec4 downTwo = imageLoad(inFieldTexture, downTwoPos);
    vec4 upTwo = imageLoad(inFieldTexture, upTwoPos);

    // Calculate Laplacian
    vec4 laplacian = -60.0f * current;
    laplacian += 16.0f * (leftOne + rightOne + downOne + upOne);
    laplacian -= leftTwo + rightTwo + downTwo + upTwo;
    laplacian /= 12.0f * pow(dx, 2.0f);

    // Store Laplacian
    imageStore(outLaplacianTexture, pos, laplacian);
}