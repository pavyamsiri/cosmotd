#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) readonly uniform image2D inTexture;
layout(rgba32f, binding = 1) writeonly uniform image2D outTexture;
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform float eta;
layout(location=3) uniform float lam;
layout(location=4) uniform float alpha;
layout(location=5) uniform float era;


vec4 laplacian(ivec2 pos, float dx) {
    // Need size to ensure periodic boundaries
    ivec2 size = imageSize(inTexture);
    vec4 current = imageLoad(inTexture, pos);
    // One step
    vec4 leftOne = imageLoad(inTexture, ivec2(mod(pos.x - 1, size.x), pos.y));
    vec4 rightOne = imageLoad(inTexture, ivec2(mod(pos.x + 1, size.x), pos.y));
    vec4 downOne = imageLoad(inTexture, ivec2(pos.x, mod(pos.y - 1, size.y)));
    vec4 upOne = imageLoad(inTexture, ivec2(pos.x, mod(pos.y + 1, size.y)));
    // Two steps
    vec4 leftTwo = imageLoad(inTexture, ivec2(mod(pos.x - 2, size.x), pos.y));
    vec4 rightTwo = imageLoad(inTexture, ivec2(mod(pos.x + 2, size.x), pos.y));
    vec4 downTwo = imageLoad(inTexture, ivec2(pos.x, mod(pos.y - 2, size.y)));
    vec4 upTwo = imageLoad(inTexture, ivec2(pos.x, mod(pos.y + 2, size.y)));

    vec4 result = 1/(12 * dx * dx) * (-60 * current + 16 * (leftOne + rightOne + downOne + upOne) - 1 * (leftTwo + rightTwo + downTwo + upTwo));

    return result;

}


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 field = imageLoad(inTexture, pos);
    float value = field.r;
    float velocity = field.g;
    float acceleration = field.b;
    float time = field.a;

    float nextAcceleration = laplacian(pos, dx).r - alpha * (era / time) * velocity - lam * (value * value  - eta * eta) * value;

    float nextVelocity = velocity + 0.5f * (acceleration + nextAcceleration) * dt;


    imageStore(outTexture, pos, vec4(value, nextVelocity, nextAcceleration, time));
}