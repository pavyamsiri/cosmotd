#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D realFieldTexture;
layout(rgba32f, binding = 1) readonly uniform image2D inRealLaplacianTexture;
layout(rgba32f, binding = 2) restrict uniform image2D imagFieldTexture;
layout(rgba32f, binding = 3) readonly uniform image2D inImagLaplacianTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dx;
layout(location=1) uniform float dt;
layout(location=2) uniform int era;
// Domain wall specific uniform parameters
layout(location=3) uniform float eta;
layout(location=4) uniform float lam;

const float ALPHA_2D = 2.0f;


void main() {
    // Current position
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Load the field data
    vec4 realField = imageLoad(realFieldTexture, pos);
    vec4 imagField = imageLoad(imagFieldTexture, pos);
    // Field value
    float realNextValue = realField.r;
    float imagNextValue = imagField.r;
    // Field velocity
    float realCurrentVelocity = realField.g;
    float imagCurrentVelocity = imagField.g;
    // Field acceleration
    float realAcceleration = realField.b;
    float imagAcceleration = imagField.b;
    // Time
    float nextTime = realField.a;

    // Square amplitude of complex field
    float squareAmplitude = pow(realNextValue, 2) + pow(imagNextValue, 2);

    // Evolve acceleration of real field
    // Laplacian term
    float realNextAcceleration = imageLoad(inRealLaplacianTexture, pos).r;
    // 'Damping' term
    realNextAcceleration -= ALPHA_2D * (1.0f / nextTime) * realCurrentVelocity;
    // Potential derivative
    realNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * realNextValue;

    // Evolve acceleration of imaginary field
    // Laplacian term
    float imagNextAcceleration = imageLoad(inImagLaplacianTexture, pos).r;
    // 'Damping' term
    imagNextAcceleration -= ALPHA_2D * (1.0f / nextTime) * imagCurrentVelocity;
    // Potential derivative
    imagNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * imagNextValue;

    // Evolve velocity
    float realNextVelocity = realCurrentVelocity + 0.5f * (realAcceleration + realNextAcceleration) * dt;
    float imagNextVelocity = imagCurrentVelocity + 0.5f * (imagAcceleration + imagNextAcceleration) * dt;

    // Store results
    imageStore(realFieldTexture, pos, vec4(realNextValue, realNextVelocity, realNextAcceleration, nextTime));
    imageStore(imagFieldTexture, pos, vec4(imagNextValue, imagNextVelocity, imagNextAcceleration, nextTime));
}