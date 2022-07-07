#pragma once

#include "simulation.h"
#include <imgui.h>
#include "log.h"

class DomainWallSimulation : public Simulation
{
public:
    bool isInitialised = false;

    DomainWallSimulation(float targetDX, float targetDT, float eta, float lam, float alpha, float era) : eta(eta), lam(lam), alpha(alpha), era(era)
    {
        // Set up compute shader
        Shader *firstComputeShader = new Shader("shaders/evolve_field.glsl", ShaderType::COMPUTE_SHADER);
        ComputeShaderProgram *firstComputeProgram = new ComputeShaderProgram(firstComputeShader);
        if (!firstComputeProgram->isInitialised)
        {
            return;
        }
        Shader *secondComputeShader = new Shader("shaders/evolve_velocity_acceleration.glsl", ShaderType::COMPUTE_SHADER);
        ComputeShaderProgram *secondComputeProgram = new ComputeShaderProgram(secondComputeShader);
        if (!secondComputeProgram->isInitialised)
        {
            return;
        }

        delete firstComputeShader;
        delete secondComputeShader;

        std::vector<std::shared_ptr<Texture2D>> readImages = Texture2D::loadFromCTDDFile("data/domain_walls_M200_N200_np486761876.ctdd");

        std::stringstream traceStream;
        traceStream << "The original texture is texture " << readImages[0]->textureID;
        logTrace(traceStream.str().c_str());

        dx = targetDX;
        dt = targetDT;

        // Create read and write images
        std::vector<std::shared_ptr<Texture2D>> writeImages = Texture2D::createTextures(readImages[0]->width, readImages[0]->height, readImages.size());

        // Add to vector
        for (size_t index = 0; index < readImages.size(); index++)
        {
            std::vector<std::shared_ptr<Texture2D>> fieldSet(2);
            fieldSet[0] = readImages[index];
            fieldSet[1] = writeImages[index];

            m_images.push_back(fieldSet);
        }

        m_evolveFieldProgram = firstComputeProgram;
        m_evolveVelocityAccelerationProgram = secondComputeProgram;

        logDebug("DW SIMULATION COMPLETE!");
    }

    void displayUniforms() override
    {
        ImGui::SliderFloat("dx", &dx, 0.0f, 10.0f);
        ImGui::SliderFloat("dt", &dt, 0.0f, 10.0f);
        ImGui::SliderFloat("eta", &eta, 0.0f, 10.0f);
        ImGui::SliderFloat("lam", &lam, 0.0f, 10.0f);
        ImGui::SliderFloat("alpha", &alpha, 0.0f, 10.0f);
        ImGui::SliderFloat("era", &era, 0.0f, 10.0f);
    }

    void bindUniforms() override
    {
        glUniform1f(0, dx);
        glUniform1f(1, dt);
        glUniform1f(2, eta);
        glUniform1f(3, lam);
        glUniform1f(4, alpha);
        glUniform1f(5, era);
    }

private:
    float eta;
    float lam;
    float alpha;
    float era;
};