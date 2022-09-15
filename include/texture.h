#pragma once

// Standard libraries
#include <stdint.h>
#include <vector>

// The supported texture wrap modes.
enum class TextureWrapMode
{
    REPEAT = 0,
    CLAMP_TO_EDGE,
};

// The supported texture filter methods.
enum class TextureFilterMode
{
    NEAREST = 0,
    LINEAR,
};

// The supported axes on which the texture wrap mode can be set.
enum class TextureWrapAxis
{
    U = 0,
    V,
    UV,
};

// The supported levels on which the texture filter mode can be set.
enum class TextureFilterLevel
{
    MIN = 0,
    MAG,
    MIN_MAG,
};

class Texture2D
{
public:
    // OpenGL texture ID. Default is 0 (null texture).
    uint32_t textureID = 0;
    // Texture width. Default is 0.
    uint32_t width = 0;
    // Texture height. Default is 0.
    uint32_t height = 0;

    // Default constructor
    Texture2D();
    // Destructor
    ~Texture2D()
    {
        release();
    }

    // Disallow copy constructor
    Texture2D(const Texture2D &) = delete;
    // Disallow copy assignment
    Texture2D &operator=(const Texture2D &) = delete;

    // Move constructor
    Texture2D(Texture2D &&other) : textureID(other.textureID)
    {
        // Set the texture ID of the old texture to null.
        other.textureID = 0;
    }

    // Move assignment operator
    Texture2D &operator=(Texture2D &&other)
    {
        // Check that not self assigning
        if (this != &other)
        {
            // Release texture resource
            release();
            // Swap the texture IDs
            std::swap(textureID, other.textureID);
        }

        return *this;
    }

    // Bind texture unit (bind to shader program)
    void bindUnit(uint32_t target);
    // Unbind texture unit (unbind from shader program)
    void unbindUnit(uint32_t target);

    // Bind texture (to context)
    void bind();
    // Unbind texture (to context)
    void unbind();

    // Release texture resource
    void release();

    // Change texture wrapping mode
    void setTextureWrap(TextureWrapAxis axis, TextureWrapMode mode);
    // Change texture filter mode
    void setTextureFilter(TextureFilterLevel level, TextureFilterMode mode);

    // Loads a field into a texture from a file using the cosmotd data file format.
    static std::vector<std::shared_ptr<Texture2D>> loadCTDD(const char *filePath);
    // Loads an image into a texture from a png file.
    static Texture2D *loadPNG(const char *filePath);
};