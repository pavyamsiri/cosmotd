#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) readonly uniform image2D inTexture;
layout(rgba32f, binding = 1) writeonly uniform image2D outTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform float alpha;
layout(location=3) uniform float era;
// Domain wall specific uniform parameters
layout(location=4) uniform float eta;
layout(location=5) uniform float lam;

float etaTest = 1.0f;
float lamTest = 5.0f;


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
    float nextValue = field.r;
    float currentVelocity = field.g;
    float acceleration = field.b;
    float nextTime = field.a;

    // Laplacian term
    float nextAcceleration = laplacian(pos, dx).r;
    // 'Damping' term
    nextAcceleration -= alpha * (era / nextTime) * currentVelocity;
    // Potential derivative
    nextAcceleration -= lamTest * (pow(nextValue, 2)  - pow(etaTest, 2)) * nextValue;

    float nextVelocity = currentVelocity + 0.5f * (acceleration + nextAcceleration) * pow(dt, 2);


    imageStore(outTexture, pos, vec4(nextValue, nextVelocity, nextAcceleration, nextTime));
}