#pragma once
// Standard libraries
#include <stdint.h>
#include <memory>

// External libraries
#include <glad/glad.h>

// Internal libraries
#include "texture.h"

// Wraps a OpenGL framebuffer and a texture
class Framebuffer
{
public:
    // Signals that the framebuffer was successfully initialised if true.
    bool isInitialised = false;

    // OpenGL framebuffer ID
    uint32_t framebufferID = 0;

    // Constructor
    Framebuffer(uint32_t width, uint32_t height);
    // Destructor
    ~Framebuffer();

    // Binds the framebuffer
    void bind();
    // Unbinds the framebuffer
    void unbind();

    // Returns the texture ID of the current render texture
    inline uint32_t getTextureID()
    {
        return m_RenderTexture->textureID;
    }

    // Returns the texture width of the current render texture
    inline uint32_t getTextureWidth()
    {
        return m_RenderTexture->width;
    }

    // Returns the texture height of the current render texture
    inline uint32_t getTextureHeight()
    {
        return m_RenderTexture->height;
    }

private:
    // Render texture
    std::shared_ptr<Texture2D> m_RenderTexture = nullptr;
};
