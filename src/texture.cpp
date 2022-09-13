#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <texture.h>
#include <log.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLenum convertTextureWrapModeToOpenGLEnum(TextureWrapMode mode)
{
    switch (mode)
    {
    case TextureWrapMode::REPEAT:
        return GL_REPEAT;
    case TextureWrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    }
    logError("Invalid texture wrap mode!");
    return 0;
}

GLenum convertTextureFilterModeToOpenGLEnum(TextureFilterMode mode)
{
    switch (mode)
    {
    case TextureFilterMode::NEAREST:
        return GL_NEAREST;
    case TextureFilterMode::LINEAR:
        return GL_LINEAR;
    }
    logError("Invalid texture filter mode!");
    return 0;
}

Texture2D::Texture2D()
{
    logTrace("Creating a blank 2D texture...");
    // Generate a texture ID
    glGenTextures(1, &textureID);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Default parameters
    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Log
    logDebug("Created Texture2D with ID %d", textureID);
}

void Texture2D::bind(uint32_t target)
{
    glBindTextureUnit(target, textureID);
}

void Texture2D::unbind(uint32_t target)
{
    glBindTextureUnit(target, 0);
}

void Texture2D::release()
{
    glDeleteTextures(1, &textureID);
    // Log
    logDebug("Deleted Texture2D with ID %d", textureID);
}

void Texture2D::setTextureWrap(TextureWrapAxis axis, TextureWrapMode mode)
{
    GLenum wrapMode = convertTextureWrapModeToOpenGLEnum(mode);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    switch (axis)
    {
    case TextureWrapAxis::U:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        break;
    case TextureWrapAxis::V:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        break;
    case TextureWrapAxis::UV:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        break;
    }

    // // Unbind
    // glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::setTextureFilter(TextureFilterLevel level, TextureFilterMode mode)
{
    GLenum filterMode = convertTextureFilterModeToOpenGLEnum(mode);

    switch (level)
    {
    case TextureFilterLevel::MIN:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        break;
    case TextureFilterLevel::MAG:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        break;
    case TextureFilterLevel::MIN_MAG:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
        break;
    }

    // // Unbind
    // glBindTexture(GL_TEXTURE_2D, 0);
}

// Texture data setters

std::vector<std::shared_ptr<Texture2D>> Texture2D::loadFromCTDDFile(const char *filePath)
{
    uint32_t numFields;
    uint32_t M;
    uint32_t N;
    float simulationTime;

    logTrace("Loading fields from file located at path %s", filePath);

    std::ifstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        dataFile.open(filePath, std::ios::binary);
        // Read header
        dataFile.read(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));

        // Create list of textures
        std::vector<std::shared_ptr<Texture2D>> fields(numFields);

        logTrace("Number of fields = %d", fields.size());
        // Read data
        for (int fieldIndex = 0; fieldIndex < numFields; fieldIndex++)
        {
            dataFile.read(reinterpret_cast<char *>(&M), sizeof(uint32_t));
            dataFile.read(reinterpret_cast<char *>(&N), sizeof(uint32_t));
            // TODO: Currently, the current time of the simulation is stored here, however there is no way to use it currently.
            dataFile.read(reinterpret_cast<char *>(&simulationTime), sizeof(float));
            logTrace("Field %d - M = %d, N = %d, currentTime = %f", fieldIndex, M, N, simulationTime);
            std::vector<float> textureData(M * N * 4);
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {

                    float fieldValue;
                    float fieldVelocity;

                    dataFile.read(reinterpret_cast<char *>(&fieldValue), sizeof(float));
                    dataFile.read(reinterpret_cast<char *>(&fieldVelocity), sizeof(float));

                    // Red channel - field value
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 0] = fieldValue;
                    // Green channel - field velocity
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 1] = fieldVelocity;
                    // Acceleration is initialised to zero. It needs to be initialised by the simulation itself, as the simulation
                    // parameters will affect the calculation.
                    // Blue channel - current field acceleration
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 2] = 0.0f;
                    // Alpha channel - next field acceleration
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 3] = 0.0f;
                }
            }
            fields[fieldIndex] = std::shared_ptr<Texture2D>(new Texture2D());
            fields[fieldIndex]->width = N;
            fields[fieldIndex]->height = M;

            glBindTexture(GL_TEXTURE_2D, fields[fieldIndex]->textureID);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(textureData.data()));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTextureStorage2D(fields[fieldIndex]->textureID, 1, GL_RGBA32F, N, M);
            glBindImageTexture(0, fields[fieldIndex]->textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            logTrace("Filled out texture %d out of %d", fieldIndex, fields.size());

            textureData.clear();
        }

        dataFile.close();

        return fields;
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());

        return std::vector<std::shared_ptr<Texture2D>>();
    }
}

std::vector<std::shared_ptr<Texture2D>> Texture2D::createTextures(uint32_t width, uint32_t height, uint32_t size)
{
    std::vector<std::shared_ptr<Texture2D>> fields(size);
    for (int fieldIndex = 0; fieldIndex < size; fieldIndex++)
    {
        fields[fieldIndex] = std::shared_ptr<Texture2D>(new Texture2D());
        fields[fieldIndex]->width = width;
        fields[fieldIndex]->height = height;

        glBindTexture(GL_TEXTURE_2D, fields[fieldIndex]->textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTextureStorage2D(fields[fieldIndex]->textureID, 1, GL_RGBA32F, width, height);
        glBindImageTexture(0, fields[fieldIndex]->textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    return fields;
}

Texture2D *Texture2D::loadFromPNG(const char *filePath)
{
    int width, height, bpp;
    unsigned char *data = stbi_load(filePath, &width, &height, &bpp, STBI_rgb);
    // Create new texture
    Texture2D *pngTexture = new Texture2D();
    pngTexture->setTextureWrap(TextureWrapAxis::UV, TextureWrapMode::CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return pngTexture;
}