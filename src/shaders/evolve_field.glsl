#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D fieldTexture;
layout(location=0) uniform float dt;


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(fieldTexture, pos);
    float currentValue = field.r;
    float currentVelocity = field.g;
    float currentAcceleration = field.b;
    float currentTime = field.a;

    float nextValue = currentValue + dt * (currentVelocity + 0.5f * currentAcceleration * dt);
    float nextTime = currentTime + dt;

    imageStore(fieldTexture, pos, vec4(nextValue, currentVelocity, currentAcceleration, nextTime));
}