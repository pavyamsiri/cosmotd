#pragma once
// Standard libraries
#include <vector>
#include <string>

// External libraries

// Internal libraries
#include "buffer.h"
#include "shader_program.h"
#include "texture.h"

// Supported data types for shader uniforms.
enum class UniformDataType
{
    INT = 0,
    INT2,
    INT3,
    INT4,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
};

// Helper function that returns a string representation for the given uniform data type.
static std::string convertUniformDataTypeToString(UniformDataType type)
{
    switch (type)
    {
    case UniformDataType::FLOAT:
        return "FLOAT";
    case UniformDataType::FLOAT2:
        return "FLOAT2";
    case UniformDataType::FLOAT3:
        return "FLOAT3";
    case UniformDataType::FLOAT4:
        return "FLOAT4";
    case UniformDataType::INT:
        return "INT";
    case UniformDataType::INT2:
        return "INT2";
    case UniformDataType::INT3:
        return "INT3";
    case UniformDataType::INT4:
        return "INT4";
    default:
        logError("Unknown uniform data type!");
        return "UNKNOWN";
    }
}

// Specifies the data type and range for a simulation parameter.
struct SimulationElement
{
public:
    // The data type of the parameter
    UniformDataType type;
    // The parameter name
    std::string name;

    // Parameter value range
    // Initial value
    float startValue;
    // Minimum value
    float minValue;
    // Maximum value
    float maxValue;
};

// Specifies the parameters required for a simulation.
struct SimulationLayout
{
public:
    // List of simulation parameters
    std::vector<SimulationElement> m_Elements;

    // Constructor that takes in a list of simulation elements.
    SimulationLayout(const std::initializer_list<SimulationElement> &elements) : m_Elements(elements) {}
};

// Encapsulates a classical field simulation. Uses compute shaders to carry out the numerical simulation.
class Simulation
{
public:
    // Run flag
    bool runFlag = false;
    // End time of the simulation in timesteps.
    int maxTimesteps = 1000;

    // Constructor
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
          m_Layout(layout)
    {
        // Resize vectors to the correct number of fields
        m_Fields.resize(m_NumFields);
        m_LaplacianTextures.resize(m_NumFields);
        // These lists are only non-empty if there are two or more fields
        size_t numPhases = floor(m_NumFields / 2);
        m_PhaseTextures.resize(numPhases);
        m_StringTextures.resize(numPhases);
        m_StringNumbers.resize(numPhases);

        // Push uniform values
        for (const auto &element : layout.m_Elements)
        {
            switch (element.type)
            {
            case UniformDataType::FLOAT:
                m_FloatUniforms.push_back(element.startValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::FLOAT2:
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::FLOAT3:
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::FLOAT4:
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                m_FloatUniforms.push_back(element.startValue);
                break;
            case UniformDataType::INT:
                m_IntUniforms.push_back((int)element.startValue);
                break;
            // TODO: Vectors might have different initial values. How do we encode that though?
            case UniformDataType::INT2:
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                break;
            case UniformDataType::INT3:
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                break;
            case UniformDataType::INT4:
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                m_IntUniforms.push_back((int)element.startValue);
                break;
            default:
                logWarning(
                    "The given uniform data type %s for the uniform named %s is invalid!",
                    convertUniformDataTypeToString(element.type).c_str(),
                    element.name.c_str());
                break;
            }
        }
    }
    // Destructor
    ~Simulation();

    // Field setters
    // Sets new fields
    void setField(std::vector<std::shared_ptr<Texture2D>> startFields);
    // Resets to beginning (before simulation)
    void resetField();
    // Randomises fields
    void randomiseFields(uint32_t width, uint32_t height, uint32_t seed);

    // Saves fields as ctdd files
    void saveFields(const char *filePath);
    // Saves phases as ctdd files
    void savePhases(const char *filePath);
    // Saves string numbers as a data file
    void saveStringNumbers(const char *filePath);

    // Runs a number of random trials and saves the string numbers for each timestep to the data folder
    void runRandomTrials(uint32_t numTrials);

    // Updates the simulation by one timestep
    void update();

    // Binds the simulation parameters as uniforms
    void bindUniforms();

    // Renders a UI that allows users to change simulation parameters
    void onUIRender();

    // Texture getters
    // Returns the field texture at the given field index
    Texture2D *getRenderTexture(uint32_t fieldIndex);
    // Returns the currently selected field texture
    Texture2D *getCurrentRenderTexture();
    // Returns the currently selected real field texture
    Texture2D *getCurrentRealTexture();
    // Returns the currently selected imaginary field texture
    Texture2D *getCurrentImagTexture();
    // Returns the currently selected Laplacian texture
    Texture2D *getCurrentLaplacian();
    // Returns the currently selected phase texture
    Texture2D *getCurrentPhase();
    // Returns the currently selected strings texture
    Texture2D *getCurrentStrings();

    // Returns the maximum absolute value of the field. This is for plotting purposes.
    float getMaxValue();

    // Returns the current simulation time
    float getCurrentSimulationTime();
    // Returns the current simulation timestep
    int getCurrentSimulationTimestep();

    // Initialise the simulation by calculating and updating the acceleration.
    void initialiseSimulation();
    // Calculates the acceleration and stores it in the fourth component of the texture.
    void calculateAcceleration();
    // Updates the acceleration.
    void updateAcceleration();
    // Calculates the Laplacian into a separate texture.
    void calculateLaplacian();
    // Calculates the phase into a separate texture.
    void calculatePhase();
    // Highlights locations on the field which is next to a cosmic string.
    void detectStrings();

    // Returns the number of strings at the current timestep.
    std::vector<int> getCurrentStringNumber();
    // Returns the number of strings at the given timestep.
    int getStringNumber(size_t timestepIndex);

    // Simulation constructors
    // Standard Peccei-Quinn real scalar field (domain wall simulation).
    static Simulation *createDomainWallSimulation();
    // Standard Peccei-Quinn complex scalar field (cosmic string simulation).
    static Simulation *createCosmicStringSimulation();
    // Standard QCD axion field (single axion simulation).
    static Simulation *createSingleAxionSimulation();
    // Companion axion field (companion axion simulation).
    static Simulation *createCompanionAxionSimulation();

    // Returns true if the simulation is detecting strings.
    inline const bool hasStrings() const
    {
        return m_HasStrings;
    }

private:
    // Field data
    // Save of the original fields before simulation for rewinding purposes.
    std::vector<std::shared_ptr<Texture2D>> m_FieldSnapshot;
    // List of fields being simulated.
    std::vector<Texture2D> m_Fields;
    // Laplacians of each field
    std::vector<Texture2D> m_LaplacianTextures;
    // Phase of each pair of fields
    std::vector<Texture2D> m_PhaseTextures;
    // Location of strings for each pair of fields
    std::vector<Texture2D> m_StringTextures;
    // Number of strings for each pair of fields
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
    int m_CurrentTimestep = 1;

    // Extra simulation parameters that are non-specific
    SimulationLayout m_Layout;

    // Uniforms
    std::vector<float> m_FloatUniforms;
    std::vector<int32_t> m_IntUniforms;

    // Render field index
    int32_t m_RenderIndex = 0;

    uint32_t m_NumFields = 0;

    uint32_t m_XNumGroups = 0;
    uint32_t m_YNumGroups = 0;

    bool m_RequiresPhase = false;
    bool m_HasStrings = false;
};