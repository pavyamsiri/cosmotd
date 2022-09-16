#version 460 core
// Work group specification
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// In/Out: Phi real field texture
layout(rgba32f, binding = 0) restrict uniform image2D phiRealFieldTexture;
// In: Phi real Laplacian texture
layout(r32f, binding = 1) restrict readonly uniform image2D inPhiRealLaplacianTexture;

// In/Out: Phi imaginary field texture
layout(rgba32f, binding = 2) restrict uniform image2D phiImagFieldTexture;
// In: Phi imaginary Laplacian texture
layout(r32f, binding = 3) restrict readonly uniform image2D inPhiImagLaplacianTexture;

// In/Out: Psi real field texture
layout(rgba32f, binding = 4) restrict uniform image2D psiRealFieldTexture;
// In: Psi real Laplacian texture
layout(r32f, binding = 5) restrict readonly uniform image2D inPsiRealLaplacianTexture;

// In/Out: Psi imaginary field texture
layout(rgba32f, binding = 6) restrict uniform image2D psiImagFieldTexture;
// In: Psi imaginary Laplacian texture
layout(r32f, binding = 7) restrict readonly uniform image2D inPsiImagLaplacianTexture;

// TODO: Need to use array textures because we ran out of bind targets
// // In: Phi phase texture
// layout(r32f, binding = 8) readonly uniform image2D inPhiPhaseTexture;
// // In: Psi phase texture
// layout(r32f, binding = 9) readonly uniform image2D inPsiPhaseTexture;

// Universal simulation uniform parameters
layout(location=0) uniform float time;
layout(location=1) uniform float dt;
layout(location=2) uniform int era;
// Companion axion specific uniform parameters
layout(location=3) uniform float eta;
layout(location=4) uniform float lam;
layout(location=5) uniform float axionStrength;
layout(location=6) uniform float kappa;
layout(location=7) uniform float tGrowthScale;
layout(location=8) uniform float tGrowthLaw;
layout(location=9) uniform float sGrowthScale;
layout(location=10) uniform float sGrowthLaw;
layout(location=11) uniform float n;
layout(location=12) uniform float nPrime;
layout(location=13) uniform float m;
layout(location=14) uniform float mPrime;


const float ALPHA_2D = 2.0f;
const float PI = 3.1415926535897932384626433832795f;


void main() {
    // Current position
    ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
    // Load the field data
    vec4 phiReal = imageLoad(phiRealFieldTexture, pos);
    vec4 phiImag = imageLoad(phiImagFieldTexture, pos);
    vec4 psiReal = imageLoad(psiRealFieldTexture, pos);
    vec4 psiImag = imageLoad(psiImagFieldTexture, pos);
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

    // Square amplitude of complex field
    float phiSquareAmplitude = pow(phiRealNextValue, 2) + pow(phiImagNextValue, 2);
    float psiSquareAmplitude = pow(psiRealNextValue, 2) + pow(psiImagNextValue, 2);

    // Phases of complex field
    // float phiPhase = imageLoad(inPhiPhaseTexture, pos).r;
    // float psiPhase = imageLoad(inPsiPhaseTexture, pos).r;
    float phiPhase = atan(phiImagNextValue, phiRealNextValue);
    float psiPhase = atan(psiImagNextValue, psiRealNextValue);

    // Axion term in potential derivative bar the field value
    float firstAxionFactor = 2 * axionStrength;
    firstAxionFactor *= pow(time / tGrowthScale, tGrowthLaw);
    firstAxionFactor *= sin(n * phiPhase + nPrime * psiPhase);
    float secondAxionFactor = 2 * axionStrength * kappa;
    secondAxionFactor *= pow(time / sGrowthScale, sGrowthLaw);
    secondAxionFactor *= sin(m * phiPhase + mPrime * psiPhase);

    // Evolve acceleration of real component of phi field
    // Laplacian term
    float phiRealNextAcceleration = imageLoad(inPhiRealLaplacianTexture, pos).r;
    // 'Damping' term
    phiRealNextAcceleration -= ALPHA_2D * (era / time) * phiRealCurrentVelocity;
    // Potential derivative
    phiRealNextAcceleration -= lam * (phiSquareAmplitude - pow(eta, 2)) * phiRealNextValue;
    // Axion contribution
    phiRealNextAcceleration += n * firstAxionFactor * phiImagNextValue / phiSquareAmplitude;
    phiRealNextAcceleration += m * secondAxionFactor * phiImagNextValue / phiSquareAmplitude;

    // Evolve acceleration of imaginary component of phi field
    // Laplacian term
    float phiImagNextAcceleration = imageLoad(inPhiImagLaplacianTexture, pos).r;
    // 'Damping' term
    phiImagNextAcceleration -= ALPHA_2D * (era / time) * phiImagCurrentVelocity;
    // Potential derivative
    phiImagNextAcceleration -= lam * (phiSquareAmplitude - pow(eta, 2)) * phiImagNextValue;
    // Axion contribution
    phiImagNextAcceleration -= n * firstAxionFactor * phiRealNextValue / phiSquareAmplitude;
    phiImagNextAcceleration -= m * secondAxionFactor * phiRealNextValue / phiSquareAmplitude;
    
    // Evolve acceleration of real component of psi field
    // Laplacian term
    float psiRealNextAcceleration = imageLoad(inPsiRealLaplacianTexture, pos).r;
    // 'Damping' term
    psiRealNextAcceleration -= ALPHA_2D * (era / time) * psiRealCurrentVelocity;
    // Potential derivative
    psiRealNextAcceleration -= lam * (psiSquareAmplitude - pow(eta, 2)) * psiRealNextValue;
    // Axion contribution
    psiRealNextAcceleration += nPrime * firstAxionFactor * psiImagNextValue / psiSquareAmplitude;
    psiRealNextAcceleration += mPrime * secondAxionFactor * psiImagNextValue / psiSquareAmplitude;

    // Evolve acceleration of imaginary component of psi field
    // Laplacian term
    float psiImagNextAcceleration = imageLoad(inPsiImagLaplacianTexture, pos).r;
    // 'Damping' term
    psiImagNextAcceleration -= ALPHA_2D * (era / time) * psiImagCurrentVelocity;
    // Potential derivative
    psiImagNextAcceleration -= lam * (psiSquareAmplitude - pow(eta, 2)) * psiImagNextValue;
    // Axion contribution
    psiImagNextAcceleration -= nPrime * firstAxionFactor * psiRealNextValue / psiSquareAmplitude;
    psiImagNextAcceleration -= mPrime * secondAxionFactor * psiRealNextValue / psiSquareAmplitude;

    // Store results
    imageStore(phiRealFieldTexture, pos, vec4(phiRealNextValue, phiRealCurrentVelocity, phiRealCurrentAcceleration, phiRealNextAcceleration));
    imageStore(phiImagFieldTexture, pos, vec4(phiImagNextValue, phiImagCurrentVelocity, phiImagCurrentAcceleration, phiImagNextAcceleration));
    imageStore(psiRealFieldTexture, pos, vec4(psiRealNextValue, psiRealCurrentVelocity, psiRealCurrentAcceleration, psiRealNextAcceleration));
    imageStore(psiImagFieldTexture, pos, vec4(psiImagNextValue, psiImagCurrentVelocity, psiImagCurrentAcceleration, psiImagNextAcceleration));
}