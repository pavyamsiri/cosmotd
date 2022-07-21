#include <glad/glad.h>
#include <imgui.h>

#include "simulation.h"

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
    // Might be better to move into loop and use static to only initialise once.
    const int xNumGroups = ceil(fields[0][pingpong].width / 8);
    const int yNumGroups = ceil(fields[0][pingpong].height / 4);

    // Evolve field and time for all fields first
    for (const auto &currentField : fields)
    {
        firstPass->use();
        glUniform1f(0, dx);
        glUniform1f(1, dt);
        // Bind read image
        glBindImageTexture(0, currentField[pingpong].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        // Bind write image
        glBindImageTexture(1, currentField[!pingpong].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

        // Dispatch and barrier
        glDispatchCompute(xNumGroups, yNumGroups, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    // TODO: This is the previous implementation of velocity and acceleration evolution. Shaders are ran for each field separately.
    // // Evolve velocity and acceleration for all fields
    // for (const auto &currentField : fields)
    // {
    //     secondPass->use();
    //     glUniform1f(0, dx);
    //     glUniform1f(1, dt);
    //     glUniform1f(2, alpha);
    //     glUniform1f(3, era);
    //     // Bind the rest of the uniforms
    //     bindUniforms();

    //     // Bind read image
    //     glBindImageTexture(0, currentField[!pingpong].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    //     // Bind write image
    //     glBindImageTexture(1, currentField[pingpong].textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //     // Binding the other fields as read only
    //     // TODO: This is not really robust as this iteration has no explicit order and so it might not match up with the shader.
    //     int otherIndex = 2;
    //     for (const auto &otherField : fields)
    //     {
    //         // Skip if same
    //         if (&currentField == &otherField)
    //         {
    //             continue;
    //         }
    //         else
    //         {
    //             glBindImageTexture(otherIndex, otherField[!pingpong].textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    //             otherIndex++;
    //         }
    //     }

    //     // Dispatch and barrier
    //     glDispatchCompute(xNumGroups, yNumGroups, 1);
    //     glMemoryBarrier(GL_ALL_BARRIER_BITS);
    // }

    // TODO: This is the new implementation where we run one shader to evolve all field's acceleration and velocity.
    secondPass->use();
    glUniform1f(0, dx);
    glUniform1f(1, dt);
    glUniform1f(2, alpha);
    glUniform1f(3, era);
    // Bind the rest of the uniforms
    bindUniforms();
    uint32_t bindIndex = 0;
    for (const auto &currentField : fields)
    {
        // Bind read image
        glActiveTexture(GL_TEXTURE0);
        glBindImageTexture(bindIndex++, currentField[!pingpong].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        // Bind write image
        glActiveTexture(GL_TEXTURE1);
        glBindImageTexture(bindIndex++, currentField[pingpong].textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    // Dispatch and barrier
    glDispatchCompute(xNumGroups, yNumGroups, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // Swap buffers
    pingpong = !pingpong;
}

void Simulation::bindUniforms()
{
    // NOTE: This is hardcoded to be 4 because currently the first four uniform locations are taken by the universal parameters.
    // They might be packed together as one float4 though and so this value might need to change to 1.
    uint32_t currentLocation = 4;

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
            std::stringstream warningStream;
            warningStream << "The given uniform data type " << convertUniformDataTypeToString(element.type);
            warningStream << " for the uniform named " << element.name << " is invalid!";
            logWarning(warningStream.str().c_str());
            break;
        }
    }
}

void Simulation::onUIRender()
{
    ImGui::Checkbox("Running", &runFlag);

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
    ImGui::SliderFloat("dt", &dt, 0.1f, 10.0f);

    // TODO: This should not have intermediate values, but be a discrete choice between radiation and matter era.
    ImGui::SliderFloat("era", &era, 1.0f, 2.0f);

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
                element.minValue.intMinValue,
                element.maxValue.intMaxValue);

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
            std::stringstream warningStream;
            warningStream << "The given uniform data type " << convertUniformDataTypeToString(element.type);
            warningStream << " for the uniform named " << element.name << " is invalid!";
            logWarning(warningStream.str().c_str());
            break;
        }
    }
}

void Simulation::setField(std::vector<std::shared_ptr<Texture2D>> startFields)
{
    // Resize vector if necessary
    if (startFields.size() > fields.size())
    {
        fields.resize(startFields.size());
    }
    // TODO: This doesn't need to happen every time we set field. Maybe have two functions, one to set a new field, and one to
    // reset to the original field.
    originalFields = std::vector<std::shared_ptr<Texture2D>>(startFields);

    // Copy texture data over
    uint32_t fieldIndex = 0;
    for (const auto &currentField : startFields)
    {
        // Resize inner vector if necessary
        if (fields[fieldIndex].size() < 2)
        {
            fields[fieldIndex].resize(2);
        }

        // Set width and height for textures
        uint32_t height = currentField->height;
        uint32_t width = currentField->width;
        fields[fieldIndex][0].width = width;
        fields[fieldIndex][0].height = height;
        fields[fieldIndex][1].width = width;
        fields[fieldIndex][1].height = height;

        // Allocate data for textures
        glBindTexture(GL_TEXTURE_2D, fields[fieldIndex][0].textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glCopyImageSubData(
            currentField->textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            fields[fieldIndex][0].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            width, height, 1);

        glBindTexture(GL_TEXTURE_2D, fields[fieldIndex][1].textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glCopyImageSubData(
            currentField->textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            fields[fieldIndex][1].textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
            width, height, 1);

        fieldIndex++;
    }
}

Texture2D *Simulation::getRenderTexture(uint32_t fieldIndex)
{
    return &fields[fieldIndex][!pingpong];
}

Texture2D *Simulation::getCurrentRenderTexture()
{
    return &fields[renderIndex][!pingpong];
}