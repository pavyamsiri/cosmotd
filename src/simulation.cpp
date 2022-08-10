#include <glad/glad.h>
#include <imgui.h>
#include <sstream>
#include <cmath>

#include "simulation.h"

// TODO: Delete this
inline int modulo(int a, int b)
{
    const int result = a % b;
    return result >= 0 ? result : result + b;
}

const char *convertUniformDataTypeToString(UniformDataType type)
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
        return "UNKNOWN";
    }
}

void Simulation::update()
{
    if (!runFlag)
    {
        return;
    }

    timestep += 1;

#if 1
    // const int xNumGroups = ceil(fields[0].width / 8);
    // const int yNumGroups = ceil(fields[0].height / 4);
    static const int xNumGroups = ceil(fields[0].width / 4);
    static const int yNumGroups = ceil(fields[0].height / 4);

    // Evolve field and time for all fields first
    uint32_t fieldIndex = 0;
    for (const auto &currentField : fields)
    {
        firstPass->use();
        glUniform1f(0, dt);
        // Bind read image
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, currentField.textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        // Dispatch and barrier
        glDispatchCompute(xNumGroups, yNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        laplacianPass->use();
        glUniform1f(0, dx);
        // Bind images
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(0, currentField.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(1, laplacians[fieldIndex].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        // Dispatch and barrier
        glDispatchCompute(xNumGroups, yNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        fieldIndex++;
    }

    // This is the new implementation where we run one shader to evolve all field's acceleration and velocity.
    secondPass->use();
    glUniform1f(0, dt);
    glUniform1i(1, era);
    // Bind the rest of the uniforms
    bindUniforms();
    uint32_t bindIndex = 0;
    fieldIndex = 0;
    uint32_t activeTextureIndex = GL_TEXTURE0;
    for (const auto &currentField : fields)
    {
        // Bind field
        glActiveTexture(activeTextureIndex++);
        glBindImageTexture(bindIndex, currentField.textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        bindIndex++;
        // Bind its laplacian
        glActiveTexture(activeTextureIndex++);
        glBindImageTexture(bindIndex, laplacians[fieldIndex].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        bindIndex++;

        fieldIndex++;
    }

    // Dispatch and barrier
    glDispatchCompute(xNumGroups, yNumGroups, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

#else

    // Manually compute the fields

    int M, N;
    int miplevel = 0;
    // Load real and imaginary field data
    glBindTexture(GL_TEXTURE_2D, fields[0].textureID);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &M);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &N);

    std::vector<float> realFieldData(M * N * 4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(realFieldData.data()));

    glBindTexture(GL_TEXTURE_2D, fields[1].textureID);

    std::vector<float> imagFieldData(M * N * 4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(imagFieldData.data()));

    // Evolve the field values
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            // Real
            float currentRealValue = realFieldData[(i * 4 * N) + 4 * j + 0];
            float currentRealVelocity = realFieldData[(i * 4 * N) + 4 * j + 1];
            float currentRealAcceleration = realFieldData[(i * 4 * N) + 4 * j + 2];
            float currentTime = realFieldData[(i * 4 * N) + 4 * j + 3];

            float nextRealValue = currentRealValue + 0.1f * (currentRealVelocity + 0.5f * currentRealAcceleration * 0.1f);
            float nextTime = currentTime + 0.1f;

            realFieldData[(i * 4 * N) + 4 * j + 0] = nextRealValue;
            realFieldData[(i * 4 * N) + 4 * j + 3] = nextTime;

            // Imaginary
            float currentImagValue = imagFieldData[(i * 4 * N) + 4 * j + 0];
            float currentImagVelocity = imagFieldData[(i * 4 * N) + 4 * j + 1];
            float currentImagAcceleration = imagFieldData[(i * 4 * N) + 4 * j + 2];

            float nextImagValue = currentImagValue + 0.1f * (currentImagVelocity + 0.5f * currentImagAcceleration * 0.1f);

            imagFieldData[(i * 4 * N) + 4 * j + 0] = nextImagValue;
            imagFieldData[(i * 4 * N) + 4 * j + 3] = nextTime;
        }
    }

    // Evolve the velocity and acceleration
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            // Indices
            size_t centralIndex = (i * 4 * N) + 4 * j;
            size_t downOneIndex = (modulo(i + 1, M) * 4 * N) + 4 * j;
            size_t upOneIndex = (modulo(i - 1, M) * 4 * N) + 4 * j;
            size_t downTwoIndex = (modulo(i + 2, M) * 4 * N) + 4 * j;
            size_t upTwoIndex = (modulo(i - 2, M) * 4 * N) + 4 * j;
            size_t rightOneIndex = (i * 4 * N) + 4 * modulo(j + 1, N);
            size_t leftOneIndex = (i * 4 * N) + 4 * modulo(j - 1, N);
            size_t rightTwoIndex = (i * 4 * N) + 4 * modulo(j + 2, N);
            size_t leftTwoIndex = (i * 4 * N) + 4 * modulo(j - 2, N);

            // Real laplacian
            float centralRealValue = realFieldData[centralIndex + 0];
            float downOneRealValue = realFieldData[downOneIndex + 0];
            float upOneRealValue = realFieldData[upOneIndex + 0];
            float downTwoRealValue = realFieldData[downTwoIndex + 0];
            float upTwoRealValue = realFieldData[upTwoIndex + 0];
            float rightOneRealValue = realFieldData[rightOneIndex + 0];
            float leftOneRealValue = realFieldData[leftOneIndex + 0];
            float rightTwoRealValue = realFieldData[rightTwoIndex + 0];
            float leftTwoRealValue = realFieldData[leftTwoIndex + 0];

            float realLaplacian = -60.0f * centralRealValue + 16.0f * (downOneRealValue + upOneRealValue + leftOneRealValue + rightOneRealValue) - (leftTwoRealValue + rightTwoRealValue + downTwoRealValue + upTwoRealValue);
            realLaplacian /= 12.0f;

            // Imaginary laplacian
            float centralImagValue = imagFieldData[centralIndex + 0];
            float downOneImagValue = imagFieldData[downOneIndex + 0];
            float upOneImagValue = imagFieldData[upOneIndex + 0];
            float downTwoImagValue = imagFieldData[downTwoIndex + 0];
            float upTwoImagValue = imagFieldData[upTwoIndex + 0];
            float rightOneImagValue = imagFieldData[rightOneIndex + 0];
            float leftOneImagValue = imagFieldData[leftOneIndex + 0];
            float rightTwoImagValue = imagFieldData[rightTwoIndex + 0];
            float leftTwoImagValue = imagFieldData[leftTwoIndex + 0];

            float imagLaplacian = -60.0f * centralImagValue + 16.0f * (downOneImagValue + upOneImagValue + leftOneImagValue + rightOneImagValue) - (leftTwoImagValue + rightTwoImagValue + downTwoImagValue + upTwoImagValue);
            imagLaplacian /= 12.0f;

            float squareAmplitude = pow(centralRealValue, 2) + pow(centralImagValue, 2);
            float nextTime = realFieldData[centralIndex + 3];
            static float colorAnomaly = 3.0f;
            static float axionStrength = 0.005f * 5.0f;
            static float growthScale = 75.0f;
            static float growthLaw = 2.0f;
            float axionFactor = 2 * colorAnomaly * axionStrength;
            axionFactor *= pow(nextTime / growthScale, growthLaw);
            axionFactor *= sin(colorAnomaly * atan2(centralImagValue, centralRealValue));
            axionFactor /= squareAmplitude;

            float currentRealAcceleration = realFieldData[centralIndex + 2];
            float currentRealVelocity = realFieldData[centralIndex + 1];

            float currentImagVelocity = imagFieldData[centralIndex + 1];
            float currentImagAcceleration = imagFieldData[centralIndex + 2];

            // Evolve acceleration
            // float nextRealAcceleration = realLaplacian - 2.0f * (1.0f / nextTime) * currentRealVelocity - 5.0f * (squareAmplitude - 1.0f) * centralRealValue;
            // float nextImagAcceleration = imagLaplacian - 2.0f * (1.0f / nextTime) * currentImagVelocity - 5.0f * (squareAmplitude - 1.0f) * centralImagValue;
            float nextRealAcceleration = realLaplacian - 2.0f * (1.0f / nextTime) * currentRealVelocity - 5.0f * (pow(centralRealValue, 2) - 1.0f) * centralRealValue - centralImagValue * axionFactor;
            float nextImagAcceleration = imagLaplacian - 2.0f * (1.0f / nextTime) * currentImagVelocity - 5.0f * (pow(centralImagValue, 2) - 1.0f) * centralImagValue + centralRealValue * axionFactor;

            // Evolve velocity
            float nextRealVelocity = currentRealVelocity + 0.5f * (currentRealAcceleration + nextRealAcceleration) * 0.1f;
            float nextImagVelocity = currentImagVelocity + 0.5f * (currentImagAcceleration + nextImagAcceleration) * 0.1f;

            // Set new values
            realFieldData[centralIndex + 1] = nextRealVelocity;
            realFieldData[centralIndex + 2] = nextRealAcceleration;

            imagFieldData[centralIndex + 1] = nextImagVelocity;
            imagFieldData[centralIndex + 2] = nextImagAcceleration;
        }
    }

    glBindTexture(GL_TEXTURE_2D, fields[0].textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(realFieldData.data()));
    glBindTexture(GL_TEXTURE_2D, fields[1].textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(imagFieldData.data()));

#endif
}

void Simulation::bindUniforms()
{
    // NOTE: This is hardcoded to be 4 because currently the first four uniform locations are taken by the universal parameters.
    // They might be packed together as one float4 though and so this value might need to change to 1.
    uint32_t currentLocation = 2;

    // Reset uniform indices
    uint32_t floatUniformIndex = 0;
    uint32_t intUniformIndex = 0;

    // Iterate through layout and bind uniforms accordingly
    for (const auto &element : layout.elements)
    {
        switch (element.type)
        {
        case UniformDataType::FLOAT:
            glUniform1f(currentLocation, floatUniforms[floatUniformIndex]);
            // Iterate to next float uniform value
            floatUniformIndex++;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT2:
            glUniform2f(currentLocation, floatUniforms[floatUniformIndex], floatUniforms[floatUniformIndex + 1]);
            // Skip ahead two float values
            floatUniformIndex = floatUniformIndex + 2;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT3:
            glUniform3f(
                currentLocation,
                floatUniforms[floatUniformIndex],
                floatUniforms[floatUniformIndex + 1],
                floatUniforms[floatUniformIndex + 2]);
            // Skip ahead three float values
            floatUniformIndex = floatUniformIndex + 3;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::FLOAT4:
            glUniform4f(
                currentLocation,
                floatUniforms[floatUniformIndex],
                floatUniforms[floatUniformIndex + 1],
                floatUniforms[floatUniformIndex + 2],
                floatUniforms[floatUniformIndex + 3]);
            // Skip ahead four float values
            floatUniformIndex = floatUniformIndex + 4;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT:
            glUniform1i(currentLocation, intUniforms[intUniformIndex]);
            // Iterate to next integer uniform value
            intUniformIndex++;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT2:
            glUniform2i(currentLocation, intUniforms[intUniformIndex], intUniforms[intUniformIndex + 1]);
            // Skip ahead two integer values
            intUniformIndex = intUniformIndex + 2;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT3:
            glUniform3i(
                currentLocation,
                intUniforms[intUniformIndex],
                intUniforms[intUniformIndex + 1],
                intUniforms[intUniformIndex + 2]);
            // Skip ahead three integer values
            intUniformIndex = intUniformIndex + 3;
            // Go to next uniform location
            currentLocation++;
            break;
        case UniformDataType::INT4:
            glUniform4i(
                currentLocation,
                intUniforms[intUniformIndex],
                intUniforms[intUniformIndex + 1],
                intUniforms[intUniformIndex + 2],
                intUniforms[intUniformIndex + 3]);
            // Skip ahead four integer values
            intUniformIndex = intUniformIndex + 4;
            // Go to next uniform location
            currentLocation++;
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

void Simulation::onUIRender()
{
    ImGui::Checkbox("Show Laplacian", &laplacianFlag);
    ImGui::Checkbox("Running", &runFlag);
    if (ImGui::Button("Save fields"))
    {
        int fieldNumber = 0;
        for (const auto &currentField : fields)
        {
            std::stringstream outStream;
            outStream << "data/saved_field_part" << fieldNumber << ".ctdd";
            currentField.saveField(outStream.str().c_str());

            fieldNumber++;
        }
    }
    if (ImGui::Button("Load fields"))
    {
        std::vector<std::shared_ptr<Texture2D>> loadedTextures = Texture2D::loadFromCTDDFile("data/saved_field.ctdd");
        setField(loadedTextures);
        originalFields = loadedTextures;
    }

    // Reset button
    if (ImGui::Button("Reset field"))
    {
        setField(originalFields);
    }

    // Toggle field to display if there is more than one field
    if (fields.size() > 1)
    {
        ImGui::SliderInt("Select fields", &renderIndex, 0, fields.size() - 1);
    }

    // NOTE: These min and max values have been randomly selected. Might change these later.
    ImGui::SliderFloat("dx", &dx, 0.1f, 10.0f);
    ImGui::SliderFloat("dt", &dt, 0.001f, 1.0f);

    ImGui::SliderInt("era", &era, 1, 2);

    // Initialise uniform indices
    uint32_t floatUniformIndex = 0;
    uint32_t intUniformIndex = 0;

    for (const auto &element : layout.elements)
    {
        switch (element.type)
        {
        case UniformDataType::FLOAT:
            ImGui::SliderFloat(
                element.name.c_str(),
                &floatUniforms[floatUniformIndex],
                element.minValue.floatMinValue,
                element.maxValue.floatMaxValue);

            floatUniformIndex++;

            break;
        case UniformDataType::FLOAT2:
            // TODO: SliderFloat2 just takes a float pointer which might work and take the two floats but it might also not work.
            ImGui::SliderFloat2(
                element.name.c_str(),
                &floatUniforms[floatUniformIndex],
                element.minValue.floatMinValue,
                element.maxValue.floatMaxValue);

            floatUniformIndex = floatUniformIndex + 2;

            break;
        case UniformDataType::FLOAT3:
            ImGui::SliderFloat3(
                element.name.c_str(),
                &floatUniforms[floatUniformIndex],
                element.minValue.floatMinValue,
                element.maxValue.floatMaxValue);

            floatUniformIndex = floatUniformIndex + 3;

            break;
        case UniformDataType::FLOAT4:
            ImGui::SliderFloat4(
                element.name.c_str(),
                &floatUniforms[floatUniformIndex],
                element.minValue.floatMinValue,
                element.maxValue.floatMaxValue);

            floatUniformIndex = floatUniformIndex + 3;

            break;
        case UniformDataType::INT:
            ImGui::SliderInt(
                element.name.c_str(),
                &intUniforms[intUniformIndex],
                (int)element.minValue.floatMinValue,
                (int)element.maxValue.floatMaxValue);

            intUniformIndex++;

            break;
        case UniformDataType::INT2:
            ImGui::SliderInt2(
                element.name.c_str(),
                &intUniforms[intUniformIndex],
                element.minValue.intMinValue,
                element.maxValue.intMaxValue);

            intUniformIndex = intUniformIndex + 2;

            break;
        case UniformDataType::INT3:
            ImGui::SliderInt3(
                element.name.c_str(),
                &intUniforms[intUniformIndex],
                element.minValue.intMinValue,
                element.maxValue.intMaxValue);

            intUniformIndex = intUniformIndex + 3;

            break;
        case UniformDataType::INT4:
            ImGui::SliderInt4(
                element.name.c_str(),
                &intUniforms[intUniformIndex],
                element.minValue.intMinValue,
                element.maxValue.intMaxValue);

            intUniformIndex = intUniformIndex + 4;

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

void Simulation::setField(std::vector<std::shared_ptr<Texture2D>> startFields)
{
    // Reset timestep
    timestep = 0;

    // Resize vector if necessary
    if (startFields.size() > fields.size())
    {
        fields.resize(startFields.size());
        laplacians.resize(startFields.size());
    }
    // TODO: This doesn't need to happen every time we set field. Maybe have two functions, one to set a new field, and one to
    // reset to the original field.
    originalFields = std::vector<std::shared_ptr<Texture2D>>(startFields);

    // Copy texture data over
    uint32_t fieldIndex = 0;
    for (const auto &currentField : startFields)
    {
        // Set width and height for textures
        uint32_t height = currentField->height;
        uint32_t width = currentField->width;
        fields[fieldIndex].width = width;
        fields[fieldIndex].height = height;

        // Allocate data for textures
        glBindTexture(GL_TEXTURE_2D, fields[fieldIndex].textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glCopyImageSubData(
            currentField->textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            fields[fieldIndex].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            width, height, 1);

        if (laplacians[fieldIndex].width != width && laplacians[fieldIndex].height != height)
        {
            glBindTexture(GL_TEXTURE_2D, laplacians[fieldIndex].textureID);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
            laplacians[fieldIndex].width = width;
            laplacians[fieldIndex].height = height;
        }

        fieldIndex++;
    }
}

Texture2D *Simulation::getRenderTexture(uint32_t fieldIndex)
{
    return &fields[fieldIndex];
}

Texture2D *Simulation::getCurrentRenderTexture()
{
    // if (laplacianFlag)
    // {
    //     return &laplacians[renderIndex];
    // }
    // else
    // {
    //     return &fields[renderIndex];
    // }
    return &fields[renderIndex];
}

float Simulation::getCurrentSimulationTime()
{
    return (timestep + 1) * dt;
}

int Simulation::getCurrentSimulationTimestep()
{
    return timestep;
}