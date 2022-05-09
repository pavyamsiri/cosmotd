#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D outTexture;

float dx = 1.0f;
float dt = 0.1f;

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(outTexture, pos);
    float value = field.r;
    float velocity = field.g;
    float acceleration = field.b;
    float time = field.a;

    float newValue = value + dt * (velocity + 0.5f * acceleration * dt);
    float newTime = time + dt;

    imageStore(outTexture, pos, vec4(newValue, velocity, acceleration, newTime));
}