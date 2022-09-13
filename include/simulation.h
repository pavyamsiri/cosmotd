#pragma once

#include <vector>

#include "buffer.h"
#include "shader_program.h"
#include "texture.h"

enum class UniformDataType
{
    INT = 0,
    INT2,
    INT3,
    INT4,
    // NOTE: Unsigned integers are not supported as they can not easily be exposed using ImGui.
    // UINT,
    // UINT2,
    // UINT3,
    // UINT4,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    // NOTE: Using doubles for uniforms requires an extension and so it won't be supported.
    // DOUBLE,
    // DOUBLE2,
    // DOUBLE3,
    // DOUBLE4,
    // NOTE: Matrices could be added but there are no plans to add it.
};

const char *convertUniformDataTypeToString(UniformDataType type);
struct SimulationElement
{
public:
    UniformDataType type;
    std::string name;

    // Initial value

    float startValue;

    // Min and max values
    float minValue;

    float maxValue;
};

struct SimulationLayout
{
public:
    SimulationLayout(const std::initializer_list<SimulationElement> &elements) : elements(elements) {}

    // Extra simulation parameters
    std::vector<SimulationElement> elements;
};

class Simulation
{
public:
    // Run flag
    bool runFlag = false;
    int maxTimesteps = 1000;
    std::vector<std::shared_ptr<Texture2D>> originalFields;
    std::vector<Texture2D> fields;

    Simulation(
        uint32_t numFields,
        ComputeShaderProgram *evolveFieldPass,
        ComputeShaderProgram *evolveVelocityPass,
        ComputeShaderProgram *calculateAccelerationPass,
        ComputeShaderProgram *updateAccelerationPass,
        ComputeShaderProgram *calculateLaplacianPass,
        ComputeShaderProgram *calculatePhasePass,
        bool requiresPhase,
        ComputeShaderProgram *detectStringsPass,
        bool hasStrings,
        SimulationLayout layout)
        : m_NumFields(numFields),
          m_EvolveFieldPass(evolveFieldPass),
          m_EvolveVelocityPass(evolveVelocityPass),
          m_CalculateAccelerationPass(calculateAccelerationPass),
          m_UpdateAccelerationPass(updateAccelerationPass),
          m_CalculateLaplacianPass(calculateLaplacianPass),
          m_CalculatePhasePass(calculatePhasePass),
          m_RequiresPhase(requiresPhase),
          m_DetectStringsPass(detectStringsPass),
          m_HasStrings(hasStrings),
          layout(layout)
    {
        // Resize vectors to the correct number of fields
        fields.resize(m_NumFields);
        m_LaplacianTextures.resize(m_NumFields);
        size_t numPhases = floor(m_NumFields / 2);
        m_PhaseTextures.resize(numPhases);
        m_StringTextures.resize(numPhases);
        m_StringNumbers.resize(numPhases);

        // Create uniform values
        for (const auto &element : layout.elements)
        {
            switch (element.type)
            {
            case UniformDataType::FLOAT:
                floatUniforms.push_back(element.startValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::FLOAT2:
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::FLOAT3:
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::FLOAT4:
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                floatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::INT:
                intUniforms.push_back((int)element.startValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::INT2:
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                break;
            case UniformDataType::INT3:
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                break;
            case UniformDataType::INT4:
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                intUniforms.push_back((int)element.startValue);
                break;
            default:
                logWarning(
                    "The given uniform data type %s for the uniform named %s is invalid!",
                    convertUniformDataTypeToString(element.type),
                    element.name.c_str());
                break;
            }
        }
    }
    ~Simulation();

    void setField(std::vector<std::shared_ptr<Texture2D>> startFields);
    void randomiseFields(uint32_t width, uint32_t height, uint32_t seed);

    void saveFields(const char *filePath);
    void savePhases(const char *filePath);
    void saveStringNumbers(const char *filePath);

    void runRandomTrials(uint32_t numTrials);

    void update();

    void bindUniforms();

    void onUIRender();

    Texture2D *getRenderTexture(uint32_t fieldIndex);
    Texture2D *getCurrentRenderTexture();
    Texture2D *getCurrentRealTexture();
    Texture2D *getCurrentImagTexture();
    Texture2D *getCurrentLaplacian();
    Texture2D *getCurrentPhase();
    Texture2D *getCurrentStrings();

    float getMaxValue();

    float getCurrentSimulationTime();
    int getCurrentSimulationTimestep();

    void initialiseSimulation();
    void calculateAcceleration();
    void updateAcceleration();
    void calculatePhase();
    void detectStrings();

    int getCurrentStringNumber();
    int getStringNumber(size_t stringIndex);

    static Simulation *createDomainWallSimulation();
    static Simulation *createCosmicStringSimulation();
    static Simulation *createSingleAxionSimulation();
    static Simulation *createCompanionAxionSimulation();

private:
    // Field data
    std::vector<Texture2D> m_LaplacianTextures;
    std::vector<Texture2D> m_PhaseTextures;
    std::vector<Texture2D> m_StringTextures;
    // Number of strings
    std::vector<std::vector<int>> m_StringNumbers;

    // Calculate and update field
    ComputeShaderProgram *m_EvolveFieldPass;
    // Calculate and update the velocity
    ComputeShaderProgram *m_EvolveVelocityPass;

    // Calculate acceleration
    ComputeShaderProgram *m_CalculateAccelerationPass;
    // Update acceleration
    ComputeShaderProgram *m_UpdateAccelerationPass;
    // Calculate Laplacian into new texture
    ComputeShaderProgram *m_CalculateLaplacianPass;

    // Calculate the phase if there are multiple fields
    ComputeShaderProgram *m_CalculatePhasePass;

    // Detect the strings
    ComputeShaderProgram *m_DetectStringsPass;

    // Universal parameters
    float dx = 1.0f;
    float dt = 0.1f;
    int era = 1;

    // Keep track of time
    int timestep = 1;

    // Extra simulation parameters that are non-specific
    SimulationLayout layout;

    // Uniforms
    std::vector<float> floatUniforms;
    std::vector<int32_t> intUniforms;

    // Render field index
    int32_t renderIndex = 0;

    uint32_t m_NumFields = 0;

    uint32_t m_XNumGroups = 0;
    uint32_t m_YNumGroups = 0;

    bool m_RequiresPhase = false;
    bool m_HasStrings = false;
};