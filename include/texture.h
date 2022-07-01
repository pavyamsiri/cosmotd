#pragma once

// Standard libraries
#include <stdint.h>
#include <vector>

class Texture2D
{
public:
    uint32_t textureID;
    uint32_t width;
    uint32_t height;

    // Constructor
    Texture2D();
    ~Texture2D();

    // Bind and unbind
    void bind(uint32_t target);
    void unbind(uint32_t target);

    // Initialisers
    static std::vector<std::shared_ptr<Texture2D>> loadFromCTDDFile(const char *filePath);
};