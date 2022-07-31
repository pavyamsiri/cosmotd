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
    ivec2 realPos = ivec2(gl_GlobalInvocationID.xy);
    vec4 realField = imageLoad(realFieldTexture, realPos);
    float realNextValue = realField.r;
    float realCurrentVelocity = realField.g;
    float realAcceleration = realField.b;
    float realNextTime = realField.a;

    ivec2 imagPos = ivec2(gl_GlobalInvocationID.xy);
    vec4 imagField = imageLoad(imagFieldTexture, imagPos);
    float imagNextValue = imagField.r;
    float imagCurrentVelocity = imagField.g;
    float imagAcceleration = imagField.b;
    float imagNextTime = imagField.a;

    float squareAmplitude = pow(realNextValue, 2) + pow(imagNextValue, 2);

    // Laplacian term
    float realNextAcceleration = imageLoad(inRealLaplacianTexture, realPos).r;
    // 'Damping' term
    realNextAcceleration -= ALPHA_2D * (1.0f / realNextTime) * realCurrentVelocity;
    // Potential derivative
    realNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * realNextValue;

    float realNextVelocity = realCurrentVelocity + 0.5f * (realAcceleration + realNextAcceleration) * dt;


    imageStore(realFieldTexture, realPos, vec4(realNextValue, realNextVelocity, realNextAcceleration, realNextTime));

    // Laplacian term
    float imagNextAcceleration = imageLoad(inImagLaplacianTexture, imagPos).r;
    // 'Damping' term
    imagNextAcceleration -= ALPHA_2D * (1.0f / imagNextTime) * imagCurrentVelocity;
    // Potential derivative
    imagNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * imagNextValue;

    float imagNextVelocity = imagCurrentVelocity + 0.5f * (imagAcceleration + imagNextAcceleration) * dt;


    imageStore(imagFieldTexture, imagPos, vec4(imagNextValue, imagNextVelocity, imagNextAcceleration, imagNextTime));
}