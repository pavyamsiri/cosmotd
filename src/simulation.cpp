#include <glad/glad.h>
#include <imgui.h>
#include <sstream>
#include <fstream>
#include <cmath>
#include <random>

#include "simulation.h"

Simulation::~Simulation()
{
    delete m_EvolveFieldPass;
    delete m_EvolveVelocityPass;
    delete m_CalculateAccelerationPass;
    delete m_UpdateAccelerationPass;
    delete m_CalculateLaplacianPass;
    if (!m_CalculatePhasePass)
    {
        delete m_CalculatePhasePass;
    }
}

void Simulation::update()
{
    if (m_CurrentTimestep > maxTimesteps)
    {
        runFlag = false;
    }

    if (!runFlag)
    {
        return;
    }

    // Evolve field and time for all fields first
    for (size_t fieldIndex = 0; fieldIndex < m_Fields.size(); fieldIndex++)
    {
        // Calculate and update field
        m_EvolveFieldPass->use();
        glUniform1f(0, dt);
        // Bind read image
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        // Calculate Laplacian
        m_CalculateLaplacianPass->use();
        glUniform1f(0, dx);
        // Bind images
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, m_LaplacianTextures[fieldIndex].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    // Update time
    m_CurrentTimestep += 1;

    // Calculate phase if there is more than one field
    if (m_Fields.size() > 1 && m_PhaseTextures.size() > 0)
    {
        calculatePhase();
    }
    // Detect strings if requested
    if (m_HasStrings && m_Fields.size() > 1 && m_StringTextures.size() > 0)
    {
        detectStrings();
    }

    // Calculate next acceleration
    calculateAcceleration();

    // Update velocity
    for (size_t fieldIndex = 0; fieldIndex < m_Fields.size(); fieldIndex++)
    {
        // Calculate and update the velocity
        m_EvolveVelocityPass->use();
        glUniform1f(0, dt);
        // Bind field
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    updateAcceleration();
}

void Simulation::bindUniforms()
{
    uint32_t currentLocation = 3;

    // Reset uniform indices
    uint32_t floatUniformIndex = 0;
    uint32_t intUniformIndex = 0;

    // Iterate through layout and bind uniforms accordingly
    for (const auto &element : m_Layout.m_Elements)
    {
        switch (element.type)
        {
        case UniformDataType::FLOAT:
            glUniform1f(currentLocation, m_FloatUniforms[floatUniformIndex]);
            // Iterate to next float uniform value
            floatUniformIndex++;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT2:
            glUniform2f(currentLocation, m_FloatUniforms[floatUniformIndex], m_FloatUniforms[floatUniformIndex + 1]);
            // Skip ahead two float values
            floatUniformIndex = floatUniformIndex + 2;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT3:
            glUniform3f(
                currentLocation,
                m_FloatUniforms[floatUniformIndex],
                m_FloatUniforms[floatUniformIndex + 1],
                m_FloatUniforms[floatUniformIndex + 2]);
            // Skip ahead three float values
            floatUniformIndex = floatUniformIndex + 3;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT4:
            glUniform4f(
                currentLocation,
                m_FloatUniforms[floatUniformIndex],
                m_FloatUniforms[floatUniformIndex + 1],
                m_FloatUniforms[floatUniformIndex + 2],
                m_FloatUniforms[floatUniformIndex + 3]);
            // Skip ahead four float values
            floatUniformIndex = floatUniformIndex + 4;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT:
            glUniform1i(currentLocation, m_IntUniforms[intUniformIndex]);
            // Iterate to next integer uniform value
            intUniformIndex++;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT2:
            glUniform2i(currentLocation, m_IntUniforms[intUniformIndex], m_IntUniforms[intUniformIndex + 1]);
            // Skip ahead two integer values
            intUniformIndex = intUniformIndex + 2;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT3:
            glUniform3i(
                currentLocation,
                m_IntUniforms[intUniformIndex],
                m_IntUniforms[intUniformIndex + 1],
                m_IntUniforms[intUniformIndex + 2]);
            // Skip ahead three integer values
            intUniformIndex = intUniformIndex + 3;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT4:
            glUniform4i(
                currentLocation,
                m_IntUniforms[intUniformIndex],
                m_IntUniforms[intUniformIndex + 1],
                m_IntUniforms[intUniformIndex + 2],
                m_IntUniforms[intUniformIndex + 3]);
            // Skip ahead four integer values
            intUniformIndex = intUniformIndex + 4;
            // Go to next uniform location
            currentLocation++;
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

void Simulation::onUIRender()
{
    if (ImGui::Checkbox("Running", &runFlag) && runFlag)
    {
        initialiseSimulation();
    }

    // Reset button
    if (ImGui::Button("Reset field"))
    {
        setField(m_FieldSnapshot);
    }

    // Toggle field to display if there is more than one field
    if (m_Fields.size() > 1)
    {
        ImGui::SliderInt("Select fields", &m_RenderIndex, 0, m_Fields.size() - 1);
    }

    ImGui::SliderFloat("dx", &dx, 0.1f, 10.0f);
    ImGui::SliderFloat("dt", &dt, 0.001f, 1.0f);

    ImGui::SliderInt("era", &era, 1, 2);

    // Initialise uniform indices
    uint32_t floatUniformIndex = 0;
    uint32_t intUniformIndex = 0;

    for (const auto &element : m_Layout.m_Elements)
    {
        switch (element.type)
        {
        case UniformDataType::FLOAT:
            ImGui::SliderFloat(
                element.name.c_str(),
                &m_FloatUniforms[floatUniformIndex],
                element.minValue,
                element.maxValue);

            floatUniformIndex++;

            break;
        case UniformDataType::FLOAT2:
            // NOTE: SliderFloat2 just takes a float pointer which might work and take the two floats but it might also not work.
            ImGui::SliderFloat2(
                element.name.c_str(),
                &m_FloatUniforms[floatUniformIndex],
                element.minValue,
                element.maxValue);

            floatUniformIndex = floatUniformIndex + 2;

            break;
        case UniformDataType::FLOAT3:
            ImGui::SliderFloat3(
                element.name.c_str(),
                &m_FloatUniforms[floatUniformIndex],
                element.minValue,
                element.maxValue);

            floatUniformIndex = floatUniformIndex + 3;

            break;
        case UniformDataType::FLOAT4:
            ImGui::SliderFloat4(
                element.name.c_str(),
                &m_FloatUniforms[floatUniformIndex],
                element.minValue,
                element.maxValue);

            floatUniformIndex = floatUniformIndex + 3;

            break;
        case UniformDataType::INT:
            ImGui::SliderInt(
                element.name.c_str(),
                &m_IntUniforms[intUniformIndex],
                (int)element.minValue,
                (int)element.maxValue);

            intUniformIndex++;

            break;
        case UniformDataType::INT2:
            ImGui::SliderInt2(
                element.name.c_str(),
                &m_IntUniforms[intUniformIndex],
                element.minValue,
                element.maxValue);

            intUniformIndex = intUniformIndex + 2;

            break;
        case UniformDataType::INT3:
            ImGui::SliderInt3(
                element.name.c_str(),
                &m_IntUniforms[intUniformIndex],
                element.minValue,
                element.maxValue);

            intUniformIndex = intUniformIndex + 3;

            break;
        case UniformDataType::INT4:
            ImGui::SliderInt4(
                element.name.c_str(),
                &m_IntUniforms[intUniformIndex],
                element.minValue,
                element.maxValue);

            intUniformIndex = intUniformIndex + 4;

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

void Simulation::setField(std::vector<std::shared_ptr<Texture2D>> newFields)
{
    // Check that the number of fields are the same or at least more
    if (m_NumFields > newFields.size())
    {
        logError("The number of fields to be set is lower than the simulations required amount. Aborting operation.");
        return;
    }

    logTrace("This simulation has %d number of required fields", m_NumFields);
    logTrace("The size of the fields vector is %d", m_Fields.size());
    logTrace("The size of the phase vector is %d", m_PhaseTextures.size());
    logTrace("The size of the Laplacian vector is %d", m_LaplacianTextures.size());
    logTrace("The size of the string vector is %d", m_StringTextures.size());
    logTrace("The size of the string numbers vector is %d", m_StringNumbers.size());
    if (m_StringNumbers.size() > 0)
    {
        logTrace("The size of the string numbers vector (count vector) is %d", m_StringNumbers[0].size());
    }

    // Reset timestep
    m_CurrentTimestep = 1;

    // TODO: This doesn't need to happen every time we set field. Maybe have two functions, one to set a new field, and one to
    // reset to the original field.
    m_FieldSnapshot = std::vector<std::shared_ptr<Texture2D>>(newFields);

    // Copy texture data overglClearTexImage
    for (size_t fieldIndex = 0; fieldIndex < m_Fields.size(); fieldIndex++)
    {
        logTrace("Field Index = %d", fieldIndex);
        logTrace("Source field index = %d", newFields[fieldIndex]->textureID);
        logTrace("Dest field index = %d", m_Fields[fieldIndex].textureID);
        // Set width and height for textures
        uint32_t height = newFields[fieldIndex]->height;
        uint32_t width = newFields[fieldIndex]->width;
        m_Fields[fieldIndex].width = width;
        m_Fields[fieldIndex].height = height;

        // Set work groups
        m_XNumGroups = ceil(width / 4);
        m_YNumGroups = ceil(height / 4);

        // Allocate data for textures
        glBindTexture(GL_TEXTURE_2D, m_Fields[fieldIndex].textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glCopyImageSubData(
            newFields[fieldIndex]->textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            m_Fields[fieldIndex].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            width, height, 1);

        // Resize Laplacian texture sizes if necessary
        if (m_LaplacianTextures[fieldIndex].width != width || m_LaplacianTextures[fieldIndex].height != height)
        {
            // Create new texture because old texture is of the wrong size
            m_LaplacianTextures[fieldIndex] = Texture2D();

            glBindTexture(GL_TEXTURE_2D, m_LaplacianTextures[fieldIndex].textureID);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
            m_LaplacianTextures[fieldIndex].width = width;
            m_LaplacianTextures[fieldIndex].height = height;
        }

        // Clear the Laplacian texture
        static float clearColor = 0.0f;
        glClearTexImage(m_LaplacianTextures[fieldIndex].textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, &clearColor);

        // Resize phase texture sizes if necessary
        if (m_PhaseTextures.size() > 0)
        {
            size_t phaseIndex = floor(fieldIndex / 2);
            logTrace("Resizing phase textures, index = %d", phaseIndex);
            if (m_PhaseTextures[phaseIndex].width != width || m_PhaseTextures[phaseIndex].height != height)
            {
                // Create new texture because old texture is of the wrong size
                m_PhaseTextures[phaseIndex] = Texture2D();

                glBindTexture(GL_TEXTURE_2D, m_PhaseTextures[phaseIndex].textureID);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
                m_PhaseTextures[phaseIndex].width = width;
                m_PhaseTextures[phaseIndex].height = height;
            }
            // Clear the phase texture
            glClearTexImage(m_PhaseTextures[phaseIndex].textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, &clearColor);
        }
        if (m_PhaseTextures.size() > 0)
        {
            size_t stringIndex = floor(fieldIndex / 2);
            logTrace("Resizing string textures, index = %d", stringIndex);
            if (m_StringTextures[stringIndex].width != width || m_StringTextures[stringIndex].height != height)
            {
                // Create new texture because old texture is of the wrong size
                m_StringTextures[stringIndex] = Texture2D();

                glBindTexture(GL_TEXTURE_2D, m_StringTextures[stringIndex].textureID);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
                m_StringTextures[stringIndex].width = width;
                m_StringTextures[stringIndex].height = height;
            }
            // Clear the phase texture
            glClearTexImage(m_StringTextures[stringIndex].textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, &clearColor);
        }
    }

    // Clear the string count
    for (auto &stringCount : m_StringNumbers)
    {
        stringCount.clear();
    }

    initialiseSimulation();
}

void Simulation::saveFields(const char *filePath)
{
    std::ofstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        uint32_t numFields = m_Fields.size();

        dataFile.open(filePath, std::ios::binary);
        // Write header
        dataFile.write(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));

        // Read data
        for (const auto &currentField : m_Fields)
        {
            glBindTexture(GL_TEXTURE_2D, currentField.textureID);
            int M, N;
            int miplevel = 0;
            float currentTime = getCurrentSimulationTime();
            glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &M);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &N);
            dataFile.write(reinterpret_cast<char *>(&M), sizeof(uint32_t));
            dataFile.write(reinterpret_cast<char *>(&N), sizeof(uint32_t));
            dataFile.write(reinterpret_cast<char *>(&currentTime), sizeof(float));

            std::vector<float> textureData(M * N * 4);

            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(textureData.data()));
            glBindTexture(GL_TEXTURE_2D, 0);
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {
                    size_t currentIndex = (rowIndex * 4 * N) + 4 * columnIndex;
                    float fieldValue = textureData[currentIndex + 0];
                    float fieldVelocity = textureData[currentIndex + 1];
                    dataFile.write(reinterpret_cast<char *>(&fieldValue), sizeof(float));
                    dataFile.write(reinterpret_cast<char *>(&fieldVelocity), sizeof(float));
                }
            }
        }

        dataFile.close();
        logTrace("Successfully wrote field data to binary file!");
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());
    }
}

void Simulation::savePhases(const char *filePath)
{
    std::ofstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        uint32_t numFields = m_PhaseTextures.size();

        dataFile.open(filePath, std::ios::binary);
        // Write header
        dataFile.write(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));

        // Read data
        for (const auto &currentPhase : m_PhaseTextures)
        {
            glBindTexture(GL_TEXTURE_2D, currentPhase.textureID);
            int M, N;
            int miplevel = 0;
            float currentTime = getCurrentSimulationTime();
            glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &M);
            glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &N);
            dataFile.write(reinterpret_cast<char *>(&M), sizeof(uint32_t));
            dataFile.write(reinterpret_cast<char *>(&N), sizeof(uint32_t));
            dataFile.write(reinterpret_cast<char *>(&currentTime), sizeof(float));

            std::vector<float> textureData(M * N);

            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, static_cast<void *>(textureData.data()));
            glBindTexture(GL_TEXTURE_2D, 0);
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {
                    size_t currentIndex = (rowIndex * N) + columnIndex;
                    float phaseValue = textureData[currentIndex];
                    float fillerValue = 0;
                    dataFile.write(reinterpret_cast<char *>(&phaseValue), sizeof(float));
                    dataFile.write(reinterpret_cast<char *>(&fillerValue), sizeof(float));
                }
            }
        }

        dataFile.close();
        logTrace("Successfully wrote phase data to binary file!");
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());
    }
}

void Simulation::saveStringNumbers(const char *filePath)
{
    // Need a non-zero size list
    if (m_StringNumbers.size() == 0)
    {
        return;
    }

    std::ofstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Open file
        dataFile.open(filePath, std::ios::binary);
        // Number of string fields
        uint32_t numStringFields = m_StringNumbers.size();
        dataFile.write(reinterpret_cast<char *>(&numStringFields), sizeof(uint32_t));
        // Write the number of timesteps. This should be the same across both
        uint32_t numTimesteps = m_StringNumbers[0].size();
        dataFile.write(reinterpret_cast<char *>(&numTimesteps), sizeof(uint32_t));

        for (size_t fieldIndex = 0; fieldIndex < m_StringNumbers.size(); fieldIndex++)
        {
            // Read data
            for (int stringCount : m_StringNumbers[fieldIndex])
            {
                dataFile.write(reinterpret_cast<char *>(&stringCount), sizeof(int));
            }
        }
        // Structure is: Number of string fields = n -> Number of timesteps (n of these) = m -> String counts (m of these)

        dataFile.close();
        logTrace("Successfully wrote string count data to binary file!");
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());
    }
}

Texture2D *Simulation::getRenderTexture(uint32_t fieldIndex)
{
    return &m_Fields[fieldIndex];
}

Texture2D *Simulation::getCurrentRenderTexture()
{
    return &m_Fields[m_RenderIndex];
}
Texture2D *Simulation::getCurrentRealTexture()
{
    size_t realIndex = 2 * floor(m_RenderIndex / 2);
    return &m_Fields[realIndex];
}
Texture2D *Simulation::getCurrentImagTexture()
{
    size_t imagIndex = ((int)(2 * floor(m_RenderIndex / 2) + 1)) % m_Fields.size();
    return &m_Fields[imagIndex];
}

Texture2D *Simulation::getCurrentLaplacian()
{
    return &m_LaplacianTextures[m_RenderIndex];
}

Texture2D *Simulation::getCurrentPhase()
{
    return &m_PhaseTextures[floor(m_RenderIndex / 2)];
}

Texture2D *Simulation::getCurrentStrings()
{
    return &m_StringTextures[floor(m_RenderIndex / 2)];
}

float Simulation::getMaxValue()
{
    return 1.0f;
}

float Simulation::getCurrentSimulationTime()
{
    return (m_CurrentTimestep + 1) * dt;
}

int Simulation::getCurrentSimulationTimestep()
{
    return m_CurrentTimestep;
}

Simulation *Simulation::createDomainWallSimulation()
{
    // Set up compute shader
    Shader *evolveFieldShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveFieldPass = new ComputeShaderProgram(evolveFieldShader);
    if (!evolveFieldPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveFieldShader;
    Shader *evolveVelocityShader = new Shader("shaders/evolve_velocity.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveVelocityPass = new ComputeShaderProgram(evolveVelocityShader);
    if (!evolveVelocityPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveVelocityShader;
    Shader *calculateAccelerationShader = new Shader("shaders/domain_walls.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateAccelerationPass = new ComputeShaderProgram(calculateAccelerationShader);
    if (!calculateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateAccelerationShader;
    Shader *updateAccelerationShader = new Shader("shaders/update_acceleration.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *updateAccelerationPass = new ComputeShaderProgram(updateAccelerationShader);
    if (!updateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete updateAccelerationShader;
    Shader *calculateLaplacianShader = new Shader("shaders/calculate_laplacian.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateLaplacianPass = new ComputeShaderProgram(calculateLaplacianShader);
    if (!calculateLaplacianPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateLaplacianShader;

    // Domain wall
    SimulationLayout simulationLayout = {
        {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f}};

    uint32_t numFields = 1;

    return new Simulation(
        numFields,
        evolveFieldPass,
        evolveVelocityPass,
        calculateAccelerationPass,
        updateAccelerationPass,
        calculateLaplacianPass,
        nullptr,
        false,
        nullptr,
        false,
        simulationLayout);
}

Simulation *Simulation::createCosmicStringSimulation()
{
    // Set up compute shader
    Shader *evolveFieldShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveFieldPass = new ComputeShaderProgram(evolveFieldShader);
    if (!evolveFieldPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveFieldShader;
    Shader *evolveVelocityShader = new Shader("shaders/evolve_velocity.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveVelocityPass = new ComputeShaderProgram(evolveVelocityShader);
    if (!evolveVelocityPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveVelocityShader;
    Shader *calculateAccelerationShader = new Shader("shaders/cosmic_strings.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateAccelerationPass = new ComputeShaderProgram(calculateAccelerationShader);
    if (!calculateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateAccelerationShader;
    Shader *updateAccelerationShader = new Shader("shaders/update_acceleration.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *updateAccelerationPass = new ComputeShaderProgram(updateAccelerationShader);
    if (!updateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete updateAccelerationShader;
    Shader *calculateLaplacianShader = new Shader("shaders/calculate_laplacian.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateLaplacianPass = new ComputeShaderProgram(calculateLaplacianShader);
    if (!calculateLaplacianPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateLaplacianShader;
    Shader *calculatePhaseShader = new Shader("shaders/calculate_phase.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculatePhasePass = new ComputeShaderProgram(calculatePhaseShader);
    if (!calculatePhasePass->isInitialised)
    {
        return nullptr;
    }
    delete calculatePhaseShader;
    Shader *detectStringsShader = new Shader("shaders/detect_strings.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *detectStringsPass = new ComputeShaderProgram(detectStringsShader);
    if (!detectStringsPass->isInitialised)
    {
        return nullptr;
    }
    delete detectStringsShader;

    // Cosmic string
    SimulationLayout simulationLayout = {
        {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f}};

    uint32_t numFields = 2;

    return new Simulation(
        numFields,
        evolveFieldPass,
        evolveVelocityPass,
        calculateAccelerationPass,
        updateAccelerationPass,
        calculateLaplacianPass,
        calculatePhasePass,
        false,
        detectStringsPass,
        true,
        simulationLayout);
}

Simulation *Simulation::createSingleAxionSimulation()
{
    // Set up compute shader
    Shader *evolveFieldShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveFieldPass = new ComputeShaderProgram(evolveFieldShader);
    if (!evolveFieldPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveFieldShader;
    Shader *evolveVelocityShader = new Shader("shaders/evolve_velocity.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveVelocityPass = new ComputeShaderProgram(evolveVelocityShader);
    if (!evolveVelocityPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveVelocityShader;
    Shader *calculateAccelerationShader = new Shader("shaders/single_axion.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateAccelerationPass = new ComputeShaderProgram(calculateAccelerationShader);
    if (!calculateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateAccelerationShader;
    Shader *updateAccelerationShader = new Shader("shaders/update_acceleration.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *updateAccelerationPass = new ComputeShaderProgram(updateAccelerationShader);
    if (!updateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete updateAccelerationShader;
    Shader *calculateLaplacianShader = new Shader("shaders/calculate_laplacian.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateLaplacianPass = new ComputeShaderProgram(calculateLaplacianShader);
    if (!calculateLaplacianPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateLaplacianShader;
    Shader *calculatePhaseShader = new Shader("shaders/calculate_phase.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculatePhasePass = new ComputeShaderProgram(calculatePhaseShader);
    if (!calculatePhasePass->isInitialised)
    {
        return nullptr;
    }
    delete calculatePhaseShader;
    Shader *detectStringsShader = new Shader("shaders/detect_strings.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *detectStringsPass = new ComputeShaderProgram(detectStringsShader);
    if (!detectStringsPass->isInitialised)
    {
        return nullptr;
    }
    delete detectStringsShader;

    // Single axion
    SimulationLayout simulationLayout = {
        {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f},
        {UniformDataType::INT, std::string("colorAnomaly"), (int)3, (int)1, (int)10},
        {UniformDataType::FLOAT, std::string("axionStrength"), 0.025f, 0.1f, 5.0f},
        {UniformDataType::FLOAT, std::string("growthScale"), 75.0f, 50.0f, 100.0f},
        {UniformDataType::FLOAT, std::string("growthLaw"), 2.0f, 1.0f, 7.0f},
    };

    uint32_t numFields = 2;

    return new Simulation(
        numFields,
        evolveFieldPass,
        evolveVelocityPass,
        calculateAccelerationPass,
        updateAccelerationPass,
        calculateLaplacianPass,
        calculatePhasePass,
        true,
        detectStringsPass,
        true,
        simulationLayout);
}

Simulation *Simulation::createCompanionAxionSimulation()
{
    // Set up compute shader
    Shader *evolveFieldShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveFieldPass = new ComputeShaderProgram(evolveFieldShader);
    if (!evolveFieldPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveFieldShader;
    Shader *evolveVelocityShader = new Shader("shaders/evolve_velocity.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *evolveVelocityPass = new ComputeShaderProgram(evolveVelocityShader);
    if (!evolveVelocityPass->isInitialised)
    {
        return nullptr;
    }
    delete evolveVelocityShader;
    Shader *calculateAccelerationShader = new Shader("shaders/companion_axion.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateAccelerationPass = new ComputeShaderProgram(calculateAccelerationShader);
    if (!calculateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateAccelerationShader;
    Shader *updateAccelerationShader = new Shader("shaders/update_acceleration.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *updateAccelerationPass = new ComputeShaderProgram(updateAccelerationShader);
    if (!updateAccelerationPass->isInitialised)
    {
        return nullptr;
    }
    delete updateAccelerationShader;
    Shader *calculateLaplacianShader = new Shader("shaders/calculate_laplacian.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculateLaplacianPass = new ComputeShaderProgram(calculateLaplacianShader);
    if (!calculateLaplacianPass->isInitialised)
    {
        return nullptr;
    }
    delete calculateLaplacianShader;
    Shader *calculatePhaseShader = new Shader("shaders/calculate_phase.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *calculatePhasePass = new ComputeShaderProgram(calculatePhaseShader);
    if (!calculatePhasePass->isInitialised)
    {
        return nullptr;
    }
    delete calculatePhaseShader;
    Shader *detectStringsShader = new Shader("shaders/detect_strings.glsl", ShaderType::COMPUTE_SHADER);
    ComputeShaderProgram *detectStringsPass = new ComputeShaderProgram(detectStringsShader);
    if (!detectStringsPass->isInitialised)
    {
        return nullptr;
    }
    delete detectStringsShader;

    // Companion axion
    SimulationLayout simulationLayout = {
        {UniformDataType::FLOAT, std::string("eta"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("lam"), 5.0f, 0.1f, 10.0f},
        {UniformDataType::FLOAT, std::string("axionStrength"), 0.025f, 0.1f, 5.0f},
        {UniformDataType::FLOAT, std::string("kappa"), 0.04f, 0.001f, 1.0f},
        {UniformDataType::FLOAT, std::string("tGrowthScale"), 75.0f, 50.0f, 100.0f},
        {UniformDataType::FLOAT, std::string("tGrowthLaw"), 2.0f, 1.0f, 7.0f},
        {UniformDataType::FLOAT, std::string("sGrowthScale"), 75.0f, 50.0f, 100.0f},
        {UniformDataType::FLOAT, std::string("sGrowthLaw"), 2.0f, 1.0f, 7.0f},
        {UniformDataType::FLOAT, std::string("n"), 3.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("nPrime"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("m"), 1.0f, 0.0f, 10.0f},
        {UniformDataType::FLOAT, std::string("mPrime"), 1.0f, 0.0f, 10.0f},
    };

    uint32_t numFields = 4;

    return new Simulation(
        numFields,
        evolveFieldPass,
        evolveVelocityPass,
        calculateAccelerationPass,
        updateAccelerationPass,
        calculateLaplacianPass,
        calculatePhasePass,
        true,
        detectStringsPass,
        true,
        simulationLayout);
}

void Simulation::calculatePhase()
{
    // Bind two textures at once and calculate the phase
    for (size_t phaseIndex = 0; phaseIndex < m_PhaseTextures.size(); phaseIndex++)
    {
        m_CalculatePhasePass->use();
        // Real part
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[(size_t)2 * phaseIndex].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Imaginary part
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, m_Fields[(size_t)2 * phaseIndex + 1].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Output phase texture
        glActiveTexture(GL_TEXTURE2);
        glBindImageTexture(2, m_PhaseTextures[phaseIndex].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
}

void Simulation::detectStrings()
{
    // Bind two textures at once and detect the strings
    for (size_t stringIndex = 0; stringIndex < m_StringTextures.size(); stringIndex++)
    {
        m_DetectStringsPass->use();
        // Real part
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[(size_t)2 * stringIndex].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Imaginary part
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, m_Fields[(size_t)2 * stringIndex + 1].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Output string texture
        glActiveTexture(GL_TEXTURE2);
        glBindImageTexture(2, m_StringTextures[stringIndex].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        // Store the string count
        m_StringNumbers[stringIndex].push_back(getStringNumber(stringIndex));
    }
}

void Simulation::calculateAcceleration()
{
    // Calculate the acceleration
    m_CalculateAccelerationPass->use();
    glUniform1f(0, m_CurrentTimestep * dt);
    glUniform1f(1, dt);
    glUniform1i(2, era);
    bindUniforms();
    uint32_t bindIndex = 0;
    uint32_t activeTextureIndex = GL_TEXTURE0;
    for (size_t fieldIndex = 0; fieldIndex < m_Fields.size(); fieldIndex++)
    {
        // Bind field
        glActiveTexture(activeTextureIndex++);
        glBindImageTexture(bindIndex++, m_Fields[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        // Bind its Laplacian
        glActiveTexture(activeTextureIndex++);
        glBindImageTexture(bindIndex++, m_LaplacianTextures[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    }

    // Bind phases if they exist
    if (m_RequiresPhase && m_PhaseTextures.size() > 0)
    {
        for (const auto &phaseTexture : m_PhaseTextures)
        {
            // Bind its phase
            glActiveTexture(activeTextureIndex++);
            glBindImageTexture(bindIndex++, phaseTexture.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
        }
    }

    // Dispatch and barrier
    glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Simulation::updateAcceleration()
{
    // Update acceleration
    for (size_t fieldIndex = 0; fieldIndex < m_Fields.size(); fieldIndex++)
    {
        // Now acceleration can be updated
        m_UpdateAccelerationPass->use();
        glUniform1f(0, dt);
        // Bind field
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, m_Fields[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        // Dispatch and barrier
        glDispatchCompute(m_XNumGroups, m_YNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
}

void Simulation::initialiseSimulation()
{
    // Calculate phase if needed
    if (m_Fields.size() > 1)
    {
        calculatePhase();
    }

    // Update acceleration but not value or velocity
    calculateAcceleration();
    updateAcceleration();
}

int Simulation::getStringNumber(size_t stringIndex)
{
    // If index out of bounds return 0.
    if (stringIndex >= m_StringTextures.size() || stringIndex < 0)
    {
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, m_StringTextures[stringIndex].textureID);
    int M, N;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &M);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &N);

    std::vector<float> textureData(M * N);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, static_cast<void *>(textureData.data()));
    glBindTexture(GL_TEXTURE_2D, 0);

    int stringNumber = 0;

    for (int rowIndex = 0; rowIndex < M; rowIndex++)
    {
        for (int columnIndex = 0; columnIndex < N; columnIndex++)
        {
            size_t currentIndex = (rowIndex * N) + columnIndex;
            float stringValue = textureData[currentIndex];
            stringNumber += abs(int(stringValue));
        }
    }

    return stringNumber;
}

std::vector<int> Simulation::getCurrentStringNumber()
{
    size_t stringIndex = floor(m_RenderIndex / 2);
    if (m_StringNumbers.size() > 0)
    {
        std::vector<int> result;
        for (const auto &currentStringVector : m_StringNumbers)
        {
            if (currentStringVector.size() > 0)
            {
                result.push_back(currentStringVector.back());
            }
        }
        return result;
    }
    else
    {
        return std::vector<int>();
    }
}

void Simulation::randomiseFields(uint32_t width, uint32_t height, uint32_t seed)
{
    std::default_random_engine seedGenerator;
    seedGenerator.seed(seed);
    std::uniform_int_distribution<uint32_t> seedDistribution(0, UINT32_MAX);

    // Create new fields
    std::vector<std::shared_ptr<Texture2D>> newFields(m_NumFields);

    for (size_t fieldIndex = 0; fieldIndex < newFields.size(); fieldIndex++)
    {
        // Random generator
        std::default_random_engine valueGenerator;
        uint32_t currentSeed = seedDistribution(seedGenerator);
        valueGenerator.seed(currentSeed);
        std::normal_distribution<float> distribution(0.0f, 1.0f);

        std::vector<float> textureData(height * width * 4);
        for (int rowIndex = 0; rowIndex < height; rowIndex++)
        {
            for (int columnIndex = 0; columnIndex < width; columnIndex++)
            {
                // Red channel - field value
                textureData[(rowIndex * 4 * width) + 4 * columnIndex + 0] = 0.1f * distribution(valueGenerator);
                // Green channel - field velocity
                textureData[(rowIndex * 4 * width) + 4 * columnIndex + 1] = 0.0f;
                // Acceleration is initialised to zero. It needs to be initialised by the simulation itself, as the simulation
                // parameters will affect the calculation.
                // Blue channel - current field acceleration
                textureData[(rowIndex * 4 * width) + 4 * columnIndex + 2] = 0.0f;
                // Alpha channel - next field acceleration
                textureData[(rowIndex * 4 * width) + 4 * columnIndex + 3] = 0.0f;
            }
        }

        Texture2D *fieldTexture = new Texture2D();
        fieldTexture->setTextureWrap(TextureWrapAxis::UV, TextureWrapMode::REPEAT);
        fieldTexture->setTextureFilter(TextureFilterLevel::MIN_MAG, TextureFilterMode::LINEAR);
        newFields[fieldIndex] = std::shared_ptr<Texture2D>(fieldTexture);
        newFields[fieldIndex]->width = width;
        newFields[fieldIndex]->height = height;

        glBindTexture(GL_TEXTURE_2D, newFields[fieldIndex]->textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(textureData.data()));
        glGenerateMipmap(GL_TEXTURE_2D);

        glTextureStorage2D(newFields[fieldIndex]->textureID, 1, GL_RGBA32F, width, height);
        glBindImageTexture(0, newFields[fieldIndex]->textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        textureData.clear();
    }

    // Set the new fields
    setField(newFields);
}

void Simulation::runRandomTrials(uint32_t numTrials)
{
    for (size_t trialIndex = 0; trialIndex < numTrials; trialIndex++)
    {
        randomiseFields(256, 256, trialIndex);
        logInfo("Beginning trial %d", trialIndex);
        runFlag = true;

        for (size_t timestepIndex = 0; timestepIndex < maxTimesteps; timestepIndex++)
        {
            update();
        }

        std::stringstream nameStream;
        nameStream << "data/string_count_trial" << trialIndex << ".data";

        saveStringNumbers(nameStream.str().c_str());
    }
    logInfo("Trials are complete!");
}