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
    Simulation(uint32_t numFields, ComputeShaderProgram *firstPass, ComputeShaderProgram *secondPass, SimulationLayout layout)
        : firstPass(firstPass), secondPass(secondPass), layout(layout)
    {
        // Initialise vector to store textures
        for (int i = 0; i < numFields; i++)
        {
            // Each field has two buffers to swap between
            fields.push_back(std::vector<Texture2D>(2));
        }

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
                intUniforms.push_back(element.startValue.intStartValue);
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
                std::stringstream warningStream;
                warningStream << "The given uniform data type " << convertUniformDataTypeToString(element.type);
                warningStream << " for the uniform named " << element.name << " is invalid!";
                logWarning(warningStream.str().c_str());
                break;
            }
        }
    }
    ~Simulation();

    void setField(std::vector<std::shared_ptr<Texture2D>> startFields);

    void update();

    void bindUniforms();

    void onUIRender();

    Texture2D *getRenderTexture(uint32_t fieldIndex);
    Texture2D *getCurrentRenderTexture();

private:
    // Field data
    std::vector<std::shared_ptr<Texture2D>> originalFields;
    std::vector<std::vector<Texture2D>> fields;

    ComputeShaderProgram *firstPass;
    ComputeShaderProgram *secondPass;
    // Universal parameters
    float dx = 1.0f;
    float dt = 0.1f;
    float alpha = 2.0f;
    float era = 1.0f;

    // Extra simulation parameters that are non-specific
    SimulationLayout layout;

    // Uniforms
    std::vector<float> floatUniforms;
    std::vector<int32_t> intUniforms;

    // Pingpong variable
    bool pingpong = false;

    // Run flag
    bool runFlag = false;

    // Render field index
    int32_t renderIndex = 0;
};