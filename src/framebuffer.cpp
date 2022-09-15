// Standard libraries

// External libraries
#include <glad/glad.h>

// Internal libraries
#include "framebuffer.h"
#include "log.h"

Framebuffer::Framebuffer(uint32_t width, uint32_t height)
{
    logDebug("Framebuffer is being created...");
    // Generate and bind framebuffer
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

    // Create render texture
    m_RenderTexture = std::shared_ptr<Texture2D>(new Texture2D());
    m_RenderTexture->width = width;
    m_RenderTexture->height = height;
    m_RenderTexture->bind();
    m_RenderTexture->setTextureFilter(TextureFilterLevel::MIN_MAG, TextureFilterMode::LINEAR);

    // Set texture to empty image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Set renderTexture as color attachment 0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_RenderTexture->textureID, 0);

    // Use color attachment 0 to draw
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    // Check that framebuffer creation was successful
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        logError("Framebuffer with ID %d failed creation!", framebufferID);
        return;
    }
    else
    {
        isInitialised = true;
        logDebug("Framebuffer successfully created with ID %d.", framebufferID);
        return;
    }
}

Framebuffer::~Framebuffer()
{
    logDebug("Framebuffer with ID %d is being destroyed...", framebufferID);

    // Delete framebuffer
    glDeleteFramebuffers(1, &framebufferID);

    logDebug("Framebuffer with ID %d has been destroyed.", framebufferID);
}

void Framebuffer::bind()
{
    logTrace("Framebuffer with ID %d is being bound.", framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
}

void Framebuffer::unbind()
{
    logTrace("Framebuffer with ID %d is being unbound.", framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}