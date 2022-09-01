#version 460 core
// Work group specification
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;

// In/Out: Real field texture
layout(rgba32f, binding = 0) restrict uniform image2D realFieldTexture;
// In: Real Laplacian texture
layout(r32f, binding = 1) readonly uniform image2D inRealLaplacianTexture;

// In/Out: Imaginary field texture
layout(rgba32f, binding = 2) restrict uniform image2D imagFieldTexture;
// In: Imaginary Laplacian texture
layout(r32f, binding = 3) readonly uniform image2D inImagLaplacianTexture;

// In: Phase texture
layout(r32f, binding = 4) readonly uniform image2D inPhaseTexture;

// Universal simulation uniform parameters
layout(location=0) uniform float time;
layout(location=1) uniform float dt;
layout(location=2) uniform int era;
// Single axion specific uniform parameters
layout(location=3) uniform float eta;
layout(location=4) uniform float lam;
layout(location=5) uniform int colorAnomaly;
layout(location=6) uniform float axionStrength;
layout(location=7) uniform float growthScale;
layout(location=8) uniform float growthLaw;


const float ALPHA_2D = 2.0f;
const float PI = 3.1415926535897932384626433832795f;
const float EPSILON = 0.01f;


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
    // Phase
    float phase = imageLoad(inPhaseTexture, pos).r;

    // Square amplitude of complex field
    float squareAmplitude = pow(realNextValue, 2) + pow(imagNextValue, 2);
    
    // Axion term in potential derivative bar the field value
    float axionFactor = 2.0f * colorAnomaly * axionStrength * pow(time / growthScale, growthLaw) * sin(colorAnomaly * phase) / squareAmplitude;

    // Evolve acceleration of real field
    // Laplacian term
    float realNextAcceleration = imageLoad(inRealLaplacianTexture, pos).r;
    // 'Damping' term
    realNextAcceleration -= ALPHA_2D * (era / time) * realCurrentVelocity;
    // Potential derivative
    realNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * realNextValue;
    // Axion contribution
    realNextAcceleration += imagNextValue * axionFactor;

    // Evolve acceleration of imaginary field
    // Laplacian term
    float imagNextAcceleration = imageLoad(inImagLaplacianTexture, pos).r;
    // 'Damping' term
    imagNextAcceleration -= ALPHA_2D * (era / time) * imagCurrentVelocity;
    // Potential derivative
    imagNextAcceleration -= lam * (squareAmplitude - pow(eta, 2)) * imagNextValue;
    // Axion contribution
    imagNextAcceleration -= realNextValue * axionFactor;

    // Store results
    imageStore(realFieldTexture, pos, vec4(realNextValue, realCurrentVelocity, realCurrentAcceleration, realNextAcceleration));
    imageStore(imagFieldTexture, pos, vec4(imagNextValue, imagCurrentVelocity, imagCurrentAcceleration, imagNextAcceleration));
}