#version 460 core
// Work group specification
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
// In/out: Field texture
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;

// Uniforms: time interval
layout(location = 0) uniform float dt;

// PRS alpha
const float ALPHA_2D = 2.0f;


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(fieldTexture, pos);
    float nextValue = field.r;
    float currentVelocity = field.g;
    float currentAcceleration = field.b;
    float nextAcceleration = field.a;

    // Calculate next velocity
    float nextVelocity = currentVelocity + 0.5f * (currentAcceleration + nextAcceleration) * dt;

    // Update velocity
    imageStore(fieldTexture, pos, vec4(nextValue, nextVelocity, currentAcceleration, nextAcceleration));
}