#version 460 core
layout(local_size_x = 4, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) restrict uniform image2D phiRealField;
layout(rgba32f, binding = 1) readonly uniform image2D phiRealLaplacian;
layout(rgba32f, binding = 2) restrict uniform image2D phiImagField;
layout(rgba32f, binding = 3) readonly uniform image2D phiImagLaplacian;
layout(rgba32f, binding = 4) restrict uniform image2D psiRealField;
layout(rgba32f, binding = 5) readonly uniform image2D psiRealLaplacian;
layout(rgba32f, binding = 6) restrict uniform image2D psiImagField;
layout(rgba32f, binding = 7) readonly uniform image2D psiImagLaplacian;
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


void main() {
    // Current position
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Load the field data
    vec4 phiReal = imageLoad(phiRealField, pos);
    vec4 phiImag = imageLoad(phiImagField, pos);
    vec4 psiReal = imageLoad(psiRealField, pos);
    vec4 psiImag = imageLoad(psiImagField, pos);
    // Field value
    float phiRealNextValue = phiReal.r;
    float phiImagNextValue = phiImag.r;
    float psiRealNextValue = psiReal.r;
    float psiImagNextValue = psiImag.r;
    // Field velocity
    float phiRealCurrentVelocity = phiReal.g;
    float phiImagCurrentVelocity = phiImag.g;
    float psiRealCurrentVelocity = psiReal.g;
    float psiImagCurrentVelocity = psiImag.g;
    // Field acceleration
    float phiRealCurrentAcceleration = phiReal.b;
    float phiImagCurrentAcceleration = phiImag.b;
    float psiRealCurrentAcceleration = psiReal.b;
    float psiImagCurrentAcceleration = psiImag.b;
    // Time
    float nextTime = realField.a;

    // Square amplitude of complex field
    float phiSquareAmplitude = pow(phiRealNextValue, 2) + pow(phiImagNextValue, 2);
    float psiSquareAmplitude = pow(psiRealNextValue, 2) + pow(psiImagNextValue, 2);

    // Phases of complex field
    float phiPhase = atan(phiImagNextValue, phiRealNextValue);
    float psiPhase = atan(psiImagNextValue, psiRealNextValue);

    // Axion term in potential derivative bar the field value
    float axionFactor = 2 * colorAnomaly * axionStrength;
    axionFactor *= pow(nextTime / growthScale, growthLaw);
    axionFactor *= sin(colorAnomaly * atan(imagNextValue, realNextValue));
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