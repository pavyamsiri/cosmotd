#pragma once

#include <vector>

#include "buffer.h"
#include "shader_program.h"
#include "texture.h"

enum class UniformDataType
{
    NONE = 0,
    INT,
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

    union startValue
    {
        float floatStartValue;
        int32_t intStartValue;
    } startValue;

    // Min and max values
    union minValue
    {
        float floatMinValue;
        int32_t intMinValue;
    } minValue;

    union maxValue
    {
        float floatMaxValue;
        int32_t intMaxValue;
    } maxValue;
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
    std::vector<std::shared_ptr<Texture2D>> originalFields;
    std::vector<Texture2D> fields;

    Simulation(uint32_t numFields, ComputeShaderProgram *firstPass, ComputeShaderProgram *laplacianPass, ComputeShaderProgram *secondPass, SimulationLayout layout)
        : firstPass(firstPass), laplacianPass(laplacianPass), secondPass(secondPass), layout(layout)
    {
        // Resize vectors to the correct number of fields
        fields.resize(numFields);
        laplacians.resize(numFields);

        // Create uniform values
        for (const auto &element : layout.elements)
        {
            switch (element.type)
            {
            case UniformDataType::FLOAT:
                floatUniforms.push_back(element.startValue.floatStartValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::FLOAT2:
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                break;
            case UniformDataType::FLOAT3:
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                break;
            case UniformDataType::FLOAT4:
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                floatUniforms.push_back(element.startValue.floatStartValue);
                break;
            case UniformDataType::INT:
                // TODO: For some reason, integer values always get interpreted as floats and so we need to cast into integer.
                intUniforms.push_back((int)element.startValue.floatStartValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::INT2:
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
                break;
            case UniformDataType::INT3:
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
                break;
            case UniformDataType::INT4:
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
                intUniforms.push_back(element.startValue.intStartValue);
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

    void saveFields(const char *filePath);
    void savePhase(const char *filePath);

    void update();

    void bindUniforms();

    void onUIRender();

    Texture2D *getRenderTexture(uint32_t fieldIndex);
    Texture2D *getCurrentRenderTexture();
    Texture2D *getCurrentLaplacian();

    float getCurrentSimulationTime();
    int getCurrentSimulationTimestep();

    static Simulation *createDomainWallSimulation();
    static Simulation *createCosmicStringSimulation();
    static Simulation *createSingleAxionSimulation();
    static Simulation *createCompanionAxionSimulation();

private:
    // Field data
    std::vector<Texture2D> laplacians;

    ComputeShaderProgram *firstPass;
    ComputeShaderProgram *laplacianPass;
    ComputeShaderProgram *phasePass;
    ComputeShaderProgram *secondPass;
    // Universal parameters
    float dx = 1.0f;
    float dt = 0.1f;
    int era = 1;

    // Keep track of time
    int timestep = 0;

    // Extra simulation parameters that are non-specific
    SimulationLayout layout;

    // Uniforms
    std::vector<float> floatUniforms;
    std::vector<int32_t> intUniforms;

    // Render field index
    int32_t renderIndex = 0;
};