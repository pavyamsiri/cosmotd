#version 460 core
// Work group specification
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
// In/out: Field texture
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;

// Uniforms: time interval
layout(location=0) uniform float dt;


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(fieldTexture, pos);
    float currentValue = field.r;
    float currentVelocity = field.g;
    float currentAcceleration = field.b;

    // Calculate next field value
    float nextValue = currentValue + dt * (currentVelocity + 0.5f * currentAcceleration * dt);

    // Update field value
    imageStore(fieldTexture, pos, vec4(nextValue, currentVelocity, currentAcceleration, 0.0f));
}