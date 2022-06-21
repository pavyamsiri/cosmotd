#pragma once

#include <glad/glad.h>

#include <stdint.h>

class Framebuffer
{
public:
    uint32_t framebufferID;
    uint32_t renderTextureID;

    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();
};
