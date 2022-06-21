#include <glad/glad.h>
#include <framebuffer.h>
#include <intrin.h>
#include <log.h>

Framebuffer::Framebuffer(uint32_t width, uint32_t height)
{
    // Generate and bind framebuffer
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

    // Create render texture
    glGenTextures(1, &renderTextureID);
    glBindTexture(GL_TEXTURE_2D, renderTextureID);

    // Set texture to empty image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // Set up filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Set renderTexture as color attachment 9
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTextureID, 0);

    // Use color attachment 0 to draw
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    // Check that framebuffer creation was successful
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        logFatal("Failed to create framebuffer!");
        __debugbreak();
    }
}

Framebuffer::~Framebuffer()
{
    // Delete render texture
    glDeleteTextures(1, &renderTextureID);

    // Delete framebuffer
    glDeleteFramebuffers(1, &framebufferID);
}