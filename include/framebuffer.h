#pragma once

#include <glad/glad.h>

#include <stdint.h>

class Framebuffer
{
public:
    uint32_t framebufferID;
    uint32_t renderTextureID;
    uint32_t width;
    uint32_t height;
    bool isInitialised = false;

    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();
};
