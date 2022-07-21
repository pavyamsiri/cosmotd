#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) readonly uniform image2D inRealTexture;
layout(rgba32f, binding = 1) writeonly uniform image2D outRealTexture;
layout(rgba32f, binding = 2) readonly uniform image2D inImagTexture;
layout(rgba32f, binding = 3) readonly uniform image2D outImagTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform float alpha;
layout(location=3) uniform float era;
// Single axion specific uniform parameters
layout(location=4) uniform float eta;
layout(location=5) uniform float lam;
layout(location=6) uniform float colorAnomaly;
layout(location=7) uniform float axionStrength;
layout(location=8) uniform float growthScale;
layout(location=9) uniform float growthLaw;


vec4 laplacian(image2D field, ivec2 pos, float dx) {
    // Need size to ensure periodic boundaries
    ivec2 size = imageSize(field);
    vec4 This = imageLoad(field, pos);
    // One step
    vec4 leftOne = imageLoad(field, ivec2(mod(pos.x - 1, size.x), pos.y));
    vec4 rightOne = imageLoad(field, ivec2(mod(pos.x + 1, size.x), pos.y));
    vec4 downOne = imageLoad(field, ivec2(pos.x, mod(pos.y - 1, size.y)));
    vec4 upOne = imageLoad(field, ivec2(pos.x, mod(pos.y + 1, size.y)));
    // Two steps
    vec4 leftTwo = imageLoad(field, ivec2(mod(pos.x - 2, size.x), pos.y));
    vec4 rightTwo = imageLoad(field, ivec2(mod(pos.x + 2, size.x), pos.y));
    vec4 downTwo = imageLoad(field, ivec2(pos.x, mod(pos.y - 2, size.y)));
    vec4 upTwo = imageLoad(field, ivec2(pos.x, mod(pos.y + 2, size.y)));

    vec4 result = 1/(12 * dx * dx) * (-60 * This + 16 * (leftOne + rightOne + downOne + upOne) - 1 * (leftTwo + rightTwo + downTwo + upTwo));

    return result;

}


float evolveAcceleration(image2D field, ivec2 pos, float squareAmplitude, float nextValue, float otherNextValue, float currentVelocity, float nextTime, float phase) {
    float nextAcceleration = laplacian(field, pos, dx).r;
    nextAcceleration -= alpha * (era / nextTime) * currentVelocity;

    float potentialDerivative = lam * (pow(nextValue, 2) - pow(eta, 2)) * nextValue;
    potentialDerivative -= 2 * colorAnomaly * axionStrength * pow(nextTime / growthScale, growthLaw) * sin(colorAnomaly * phase) * otherNextValue / squareAmplitude;

    nextAcceleration -= potentialDerivative;

    return nextAcceleration;
}


void main() {
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    vec4 realField = imageLoad(inRealTexture, pos);
    vec4 imagField = imageLoad(inImagTexture, pos);
    float nextRealValue = realField.r;
    float nextImagValue = imagField.r;
    float currentRealVelocity = realField.g;
    float currentImagVelocity = imagField.g;
    float currentRealAcceleration = realField.b;
    float currentImagAcceleration = imagField.b;
    // These should be the same.
    float nextRealTime = realField.a;
    float nextImagTime = imagField.a;

    float squareAmplitude = pow(nextRealValue, 2) + pow(nextImageValue, 2);
    float phase = atan(nextImageValue / nextRealValue);
    float nextRealAcceleration = evolveAcceleration(inRealTexture, pos, squareAmplitude, nextRealValue, nextImagValue, currentRealVelocity, nextRealTime, phase);
    float nextImagAcceleration = evolveAcceleration(inImagTexture, pos, squareAmplitude, nextImagValue, nextRealValue, currentImagVelocity, nextImagTime, phase);

    float nextRealVelocity = currentRealVelocity + 0.5f * (currentRealAcceleration + nextRealAcceleration) * pow(dt, 2);
    float nextImagVelocity = currentImagVelocity + 0.5f * (currentImagAcceleration + nextImagAcceleration) * pow(dt, 2);

    // Write to real texture
    imageStore(outRealTexture, pos, vec4(nextRealValue, nextRealVelocity, nextRealAcceleration, nextRealTime));
    // Write to imaginary texture
    imageStore(outImagTexture, pos, vec4(nextImagValue, nextImagVelocity, nextImagAcceleration, nextImagTime));
}