#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;
layout(rgba32f, binding = 1) readonly uniform image2D inLaplacianTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform float alpha;
layout(location=3) uniform float era;
// Domain wall specific uniform parameters
layout(location=4) uniform float eta;
layout(location=5) uniform float lam;


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(fieldTexture, pos);
    float nextValue = field.r;
    float currentVelocity = field.g;
    float acceleration = field.b;
    float nextTime = field.a;

    // Laplacian term
    float nextAcceleration = imageLoad(inLaplacianTexture, pos).r;
    // 'Damping' term
    nextAcceleration -= 2.0f * (1.0f / nextTime) * currentVelocity;
    // Potential derivative
    nextAcceleration -= 5.0f * (pow(nextValue, 2)  - 1.0f) * nextValue;

    float nextVelocity = currentVelocity + 0.5f * (acceleration + nextAcceleration) * 0.01f;


    imageStore(fieldTexture, pos, vec4(nextValue, nextVelocity, nextAcceleration, nextTime));
}