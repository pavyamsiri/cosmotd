#pragma once

// Standard libraries
#include <vector>

// External libraries
#include <glad/glad.h>

// Internal libraries
#include "texture.h"
#include "shader_program.h"

class Simulation
{
public:
    virtual void displayUniforms() = 0;

    virtual void bindUniforms() = 0;

    std::vector<std::shared_ptr<Texture2D>> getRenderTextures()
    {
        // Create new vector
        std::vector<std::shared_ptr<Texture2D>> renderTextures(m_images.size());
        for (size_t index = 0; index < m_images.size(); index++)
        {
            std::stringstream traceStream;
            traceStream << "Binding texture " << m_images[index][!m_pingpong]->textureID << " as render texture";
            logTrace(traceStream.str().c_str());
            renderTextures[index] = m_images[index][!m_pingpong];
        }

        return renderTextures;
    }

    void step()
    {
        for (const auto &fieldSet : m_images)
        {
            std::stringstream firstStream;
            std::stringstream secondStream;
            std::stringstream thirdStream;
            std::stringstream fourthStream;
            // Evolve each field
            m_evolveFieldProgram->use();
            glUniform1f(0, dx);
            glUniform1f(1, dt);
            // Bind read texture
            glBindImageTexture(0, fieldSet[m_pingpong]->textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            firstStream << "Binding texture " << fieldSet[m_pingpong]->textureID << " as read texture of first stage.";
            logTrace(firstStream.str().c_str());
            // Bind write texture
            glBindImageTexture(1, fieldSet[!m_pingpong]->textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            secondStream << "Binding texture " << fieldSet[!m_pingpong]->textureID << " as write texture of first stage.";
            logTrace(secondStream.str().c_str());
            glDispatchCompute(ceil(fieldSet[m_pingpong]->width / 8), ceil(fieldSet[m_pingpong]->height / 4), 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // Evolve their velocity and acceleration
            m_evolveVelocityAccelerationProgram->use();
            bindUniforms();
            // Bind read texture
            glBindImageTexture(0, fieldSet[!m_pingpong]->textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            thirdStream << "Binding texture " << fieldSet[!m_pingpong]->textureID << " as read texture of second stage.";
            logTrace(thirdStream.str().c_str());
            // Bind write texture
            glBindImageTexture(1, fieldSet[m_pingpong]->textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            fourthStream << "Binding texture " << fieldSet[m_pingpong]->textureID << " as write texture of second stage.";
            logTrace(fourthStream.str().c_str());
            glDispatchCompute(ceil(fieldSet[m_pingpong]->width / 8), ceil(fieldSet[m_pingpong]->height / 4), 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            // Swap read and write images
            m_pingpong = !m_pingpong;
        }
    }

protected:
    float dx;
    float dt;

    // Nx2 vector - each row is a vector of 2 images used for pingpong. N == the number of fields in the simulation
    std::vector<std::vector<std::shared_ptr<Texture2D>>> m_images;
    bool m_pingpong = false;

    ComputeShaderProgram *m_evolveFieldProgram;
    ComputeShaderProgram *m_evolveVelocityAccelerationProgram;
};