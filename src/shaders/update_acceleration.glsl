#version 460 core
// Work group specification
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
// In/out: Field texture
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;

// Uniforms: time interval
layout(location=0) uniform float dt;

// PRS alpha
const float ALPHA_2D = 2.0f;


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(fieldTexture, pos);
    float nextValue = field.r;
    float nextVelocity = field.g;
    float currentAcceleration = field.b;
    float nextAcceleration = field.a;

    // Update acceleration
    imageStore(fieldTexture, pos, vec4(nextValue, nextVelocity, nextAcceleration, nextAcceleration));
}