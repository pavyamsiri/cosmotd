#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;
layout(rgba32f, binding = 1) readonly uniform image2D inLaplacianTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform int era;
// Domain wall specific uniform parameters
layout(location=3) uniform float eta;
layout(location=4) uniform float lam;

const float ALPHA_2D = 2.0f;


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
    nextAcceleration -= ALPHA_2D * (1.0f / nextTime) * currentVelocity;
    // Potential derivative
    nextAcceleration -= lam * (pow(nextValue, 2)  - pow(eta, 2)) * nextValue;

    float nextVelocity = currentVelocity + 0.5f * (acceleration + nextAcceleration) * pow(dt, 2);


    imageStore(fieldTexture, pos, vec4(nextValue, nextVelocity, nextAcceleration, nextTime));
}