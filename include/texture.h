#pragma once

#include <stdint.h>
#include <vector>

class Texture2D
{
public:
    uint32_t textureID;

    // Constructor
    Texture2D();
    ~Texture2D();

    // Initialisers
    static std::vector<Texture2D> loadFromCTDDFile(const char *filePath);
};