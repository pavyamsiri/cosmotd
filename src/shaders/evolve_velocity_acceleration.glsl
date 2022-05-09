#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D outTexture;

float dx = 1.0f;
float dt = 0.1f;
float eta = 1.0f;
float lam = 1.0f;
float alpha = 2.0f;
float era = 1.0f;

vec4 laplacian(ivec2 pos, float dx) {
    vec4 current = imageLoad(outTexture, pos);
    // One step
    vec4 leftOne = imageLoad(outTexture, ivec2(pos.x - 1, pos.y));
    vec4 rightOne = imageLoad(outTexture, ivec2(pos.x + 1, pos.y));
    vec4 downOne = imageLoad(outTexture, ivec2(pos.x, pos.y - 1));
    vec4 upOne = imageLoad(outTexture, ivec2(pos.x, pos.y + 1));
    // Two steps
    vec4 leftTwo = imageLoad(outTexture, ivec2(pos.x - 2, pos.y));
    vec4 rightTwo = imageLoad(outTexture, ivec2(pos.x + 2, pos.y));
    vec4 downTwo = imageLoad(outTexture, ivec2(pos.x, pos.y - 2));
    vec4 upTwo = imageLoad(outTexture, ivec2(pos.x, pos.y + 2));

    vec4 result = 1/(12 * dx * dx) * (-60 * current + 16 * (leftOne + rightOne + downOne + upOne) - 1 * (leftTwo + rightTwo + downTwo + upTwo));

    return result;

}

void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(outTexture, pos);
    float value = field.r;
    float velocity = field.g;
    float acceleration = field.b;
    float time = field.a;

    float nextAcceleration = laplacian(pos, dx).r - alpha * (era / time) * velocity - lam * (value * value  - eta * eta) * value;

    float nextVelocity = velocity + 0.5f * (acceleration + nextAcceleration) * dt;


    imageStore(outTexture, pos, vec4(value, nextVelocity, nextAcceleration, time));
}