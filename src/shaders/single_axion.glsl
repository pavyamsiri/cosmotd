#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D realFieldTexture;
layout(rgba32f, binding = 1) readonly uniform image2D inRealLaplacianTexture;
layout(rgba32f, binding = 2) restrict uniform image2D imagFieldTexture;
layout(rgba32f, binding = 3) readonly uniform image2D inImagLaplacianTexture;
// Universal simulation uniform parameters
layout(location=0) uniform float dt;
layout(location=1) uniform int era;
// Single axion specific uniform parameters
layout(location=2) uniform float eta;
layout(location=3) uniform float lam;
layout(location=4) uniform int colorAnomaly;
layout(location=5) uniform float axionStrength;
layout(location=6) uniform float growthScale;
layout(location=7) uniform float growthLaw;


const float ALPHA_2D = 2.0f;
const float PI = 3.1415926535897932384626433832795f;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}


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
    float realCurrentAcceleration = realField.b;
    float imagCurrentAcceleration = imagField.b;
    // Time
    float nextTime = realField.a;

    // Square amplitude of complex field
    float squareAmplitude = pow(realNextValue, 2) + pow(imagNextValue, 2);
    // Phase
    float phase = clamp(atan(imagNextValue, realNextValue), -PI, +PI);
    
    // Axion term in potential derivative bar the field value
    float axionFactor = 2 * colorAnomaly * axionStrength;
    axionFactor *= pow(nextTime / growthScale, growthLaw);
    axionFactor *= sin(colorAnomaly * phase);
    axionFactor /= squareAmplitude;

    // Evolve acceleration of real field
    // Laplacian term
    float realNextAcceleration = imageLoad(inRealLaplacianTexture, pos).r;
    // 'Damping' term
    realNextAcceleration -= ALPHA_2D * (era / nextTime) * realCurrentVelocity;
    // Potential derivative
    realNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * realNextValue;
    // Axion contribution
    realNextAcceleration -= imagNextValue * axionFactor;

    // Evolve acceleration of imaginary field
    // Laplacian term
    float imagNextAcceleration = imageLoad(inImagLaplacianTexture, pos).r;
    // 'Damping' term
    imagNextAcceleration -= ALPHA_2D * (era / nextTime) * imagCurrentVelocity;
    // Potential derivative
    imagNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * imagNextValue;
    // Axion contribution
    imagNextAcceleration += realNextValue * axionFactor;

    // Evolve velocity
    float realNextVelocity = realCurrentVelocity + 0.5f * (realCurrentAcceleration + realNextAcceleration) * dt;
    float imagNextVelocity = imagCurrentVelocity + 0.5f * (imagCurrentAcceleration + imagNextAcceleration) * dt;

    // Store results
    imageStore(realFieldTexture, pos, vec4(realNextValue, realNextVelocity, realNextAcceleration, nextTime));
    imageStore(imagFieldTexture, pos, vec4(imagNextValue, imagNextVelocity, imagNextAcceleration, nextTime));
}