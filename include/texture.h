#pragma once

// Standard libraries
#include <stdint.h>
#include <vector>

class Texture2D
{
public:
    // Initialise to null texture
    uint32_t textureID = 0;
    // Dimensions
    uint32_t width = 0;
    uint32_t height = 0;

    // Constructor
    Texture2D();
    ~Texture2D()
    {
        release();
    }

    // Disallow copy constructor/assignment
    Texture2D(const Texture2D &) = delete;
    Texture2D &operator=(const Texture2D &) = delete;

    // Move constructor
    Texture2D(Texture2D &&other) : textureID(other.textureID)
    {
        // Set the texture ID of the old texture to null.
        other.textureID = 0;
    }

    Texture2D &operator=(Texture2D &&other)
    {
        // ALWAYS check for self-assignment.
        if (this != &other)
        {
            release();
            // obj_ is now 0.
            std::swap(textureID, other.textureID);
        }
    }

    // Bind and unbind
    void bind(uint32_t target);
    void unbind(uint32_t target);

    // Release texture
    void release();

    // Save texture as .ctdd
    const void saveField(const char *filePath) const;

    // Initialisers
    static std::vector<std::shared_ptr<Texture2D>> loadFromCTDDFile(const char *filePath);
    static std::vector<std::shared_ptr<Texture2D>> createTextures(uint32_t width, uint32_t height, uint32_t size);
};