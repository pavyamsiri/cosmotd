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
// Companion axion specific uniform parameters
layout(location=2) uniform float eta;
layout(location=3) uniform float lam;
layout(location=4) uniform float axionStrength;
layout(location=5) uniform float kappa;
layout(location=6) uniform float growthScale;
layout(location=7) uniform float growthLaw;
layout(location=8) uniform float n;
layout(location=9) uniform float nPrime;
layout(location=10) uniform float m;
layout(location=11) uniform float mPrime;


const float ALPHA_2D = 2.0f;
const float PI = 3.1415926535897932384626433832795f;


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
    float nextTime = phiReal.a;

    // Square amplitude of complex field
    float phiSquareAmplitude = pow(phiRealNextValue, 2) + pow(phiImagNextValue, 2) + 0.000001f;
    float psiSquareAmplitude = pow(psiRealNextValue, 2) + pow(psiImagNextValue, 2) + 0.000001f;

    // Phases of complex field
    float phiPhase = clamp(atan(phiImagNextValue, phiRealNextValue), -PI, PI);
    float psiPhase = clamp(atan(psiImagNextValue, psiRealNextValue), -PI, PI);

    // Axion term in potential derivative bar the field value
    float firstAxionFactor = 2 * axionStrength;
    firstAxionFactor *= pow(nextTime / growthScale, growthLaw);
    firstAxionFactor *= sin(n * phiPhase + nPrime * psiPhase);
    float secondAxionFactor = 2 * axionStrength * kappa;
    secondAxionFactor *= pow(nextTime / growthScale, growthLaw);
    secondAxionFactor *= sin(m * phiPhase + mPrime * psiPhase);

    // Evolve acceleration of real component of phi field
    // Laplacian term
    float phiRealNextAcceleration = imageLoad(phiRealLaplacian, pos).r;
    // 'Damping' term
    phiRealNextAcceleration -= ALPHA_2D * (era / nextTime) * phiRealCurrentVelocity;
    // Potential derivative
    phiRealNextAcceleration -= lam * (phiSquareAmplitude - pow(eta, 2)) * phiRealNextValue;
    // Axion contribution
    phiRealNextAcceleration += n * firstAxionFactor * phiImagNextValue / phiSquareAmplitude;
    phiRealNextAcceleration += m * secondAxionFactor * phiImagNextValue / phiSquareAmplitude;

    // Evolve acceleration of imaginary component of phi field
    // Laplacian term
    float phiImagNextAcceleration = imageLoad(phiImagLaplacian, pos).r;
    // 'Damping' term
    phiImagNextAcceleration -= ALPHA_2D * (era / nextTime) * phiImagCurrentVelocity;
    // Potential derivative
    phiImagNextAcceleration -= lam * (phiSquareAmplitude - pow(eta, 2)) * phiImagNextValue;
    // Axion contribution
    phiImagNextAcceleration -= n * firstAxionFactor * phiRealNextValue / phiSquareAmplitude;
    phiImagNextAcceleration -= m * secondAxionFactor * phiRealNextValue / phiSquareAmplitude;
    
    // Evolve acceleration of real component of psi field
    // Laplacian term
    float psiRealNextAcceleration = imageLoad(psiRealLaplacian, pos).r;
    // 'Damping' term
    psiRealNextAcceleration -= ALPHA_2D * (era / nextTime) * psiRealCurrentVelocity;
    // Potential derivative
    psiRealNextAcceleration -= lam * (psiSquareAmplitude - pow(eta, 2)) * psiRealNextValue;
    // Axion contribution
    psiRealNextAcceleration += nPrime * firstAxionFactor * psiImagNextValue / psiSquareAmplitude;
    psiRealNextAcceleration += mPrime * secondAxionFactor * psiImagNextValue / psiSquareAmplitude;

    // Evolve acceleration of imaginary component of psi field
    // Laplacian term
    float psiImagNextAcceleration = imageLoad(psiImagLaplacian, pos).r;
    // 'Damping' term
    psiImagNextAcceleration -= ALPHA_2D * (era / nextTime) * psiImagCurrentVelocity;
    // Potential derivative
    psiImagNextAcceleration -= lam * (psiSquareAmplitude - pow(eta, 2)) * psiImagNextValue;
    // Axion contribution
    psiImagNextAcceleration -= nPrime * firstAxionFactor * psiRealNextValue / psiSquareAmplitude;
    psiImagNextAcceleration -= mPrime * secondAxionFactor * psiRealNextValue / psiSquareAmplitude;

    // Evolve velocity
    float phiRealNextVelocity = phiRealCurrentVelocity + 0.5f * (phiRealCurrentAcceleration + phiRealNextAcceleration) * dt;
    float phiImagNextVelocity = phiImagCurrentVelocity + 0.5f * (phiImagCurrentAcceleration + phiImagNextAcceleration) * dt;
    float psiRealNextVelocity = psiRealCurrentVelocity + 0.5f * (psiRealCurrentAcceleration + psiRealNextAcceleration) * dt;
    float psiImagNextVelocity = psiImagCurrentVelocity + 0.5f * (psiImagCurrentAcceleration + psiImagNextAcceleration) * dt;

    // Store results
    imageStore(phiRealField, pos, vec4(phiRealNextValue, phiRealNextVelocity, phiRealNextAcceleration, nextTime));
    imageStore(phiImagField, pos, vec4(phiImagNextValue, phiImagNextVelocity, phiImagNextAcceleration, nextTime));
    imageStore(psiRealField, pos, vec4(psiRealNextValue, psiRealNextVelocity, psiRealNextAcceleration, nextTime));
    imageStore(psiImagField, pos, vec4(psiImagNextValue, psiImagNextVelocity, psiImagNextAcceleration, nextTime));
}