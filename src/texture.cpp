// Standard libraries
#include <fstream>
#include <sstream>

// External libraries
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Internal libraries
#include "log.h"
#include "texture.h"

// Helper function that converts a TextureWrapMode to its corresponding GLenum
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

// Helper function that converts a TextureFilterMode to its corresponding GLenum
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
    logDebug("Blank Texture2D is being created...");
    // Generate ID
    glGenTextures(1, &textureID);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // TODO: This is probably not necessary, and it would be better to force users to be clear of the modes
    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    logDebug("Texture2D successfully created with ID %d.", textureID);
}

void Texture2D::bindUnit(uint32_t target)
{
    logTrace("Binding Texture2D with ID %d to unit %d.", textureID, target);
    glBindTextureUnit(target, textureID);
}

void Texture2D::unbindUnit(uint32_t target)
{
    logTrace("Unbinding Texture2D with ID %d from unit %d.", textureID, target);
    glBindTextureUnit(target, 0);
}

void Texture2D::bind()
{
    logTrace("Binding Texture2D with ID %d.", textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture2D::unbind()
{
    logTrace("Unbinding Texture2D with ID %d.", textureID);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::release()
{
    logDebug("Texture2D with ID %d is being destroyed...", textureID);
    glDeleteTextures(1, &textureID);
    logDebug("Texture2D with ID %d has been destroyed.", textureID);
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
}

void Texture2D::setTextureFilter(TextureFilterLevel level, TextureFilterMode mode)
{
    GLenum filterMode = convertTextureFilterModeToOpenGLEnum(mode);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

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
}

// Loading from files

std::vector<std::shared_ptr<Texture2D>> Texture2D::loadCTDD(const char *filePath)
{
    logDebug("Loading fields from CTDD file located at path %s as textures...", filePath);

    uint32_t numFields;
    uint32_t M;
    uint32_t N;
    float simulationTime;

    std::ifstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Open file
        dataFile.open(filePath, std::ios::binary);

        // Read header
        dataFile.read(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));

        // Create list of textures
        std::vector<std::shared_ptr<Texture2D>> fields(numFields);
        logTrace("File contains %d field(s).", fields.size());

        // Iterate over all fields
        for (int fieldIndex = 0; fieldIndex < numFields; fieldIndex++)
        {
            // Read field size
            dataFile.read(reinterpret_cast<char *>(&M), sizeof(uint32_t));
            dataFile.read(reinterpret_cast<char *>(&N), sizeof(uint32_t));
            // Read current simulation time
            // TODO: Currently, the current time of the simulation is stored here, however there is no way to use it currently.
            dataFile.read(reinterpret_cast<char *>(&simulationTime), sizeof(float));
            logTrace("Field %d is of size (M, N) = (%d, %d). Current time is %f", fieldIndex + 1, M, N, simulationTime);

            // Create vector to store field data
            std::vector<float> textureData(M * N * 4);

            // Iterate through field data
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {
                    // Read field value and velocity
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

            // Create new texture to store field data into
            fields[fieldIndex] = std::shared_ptr<Texture2D>(new Texture2D());
            fields[fieldIndex]->width = N;
            fields[fieldIndex]->height = M;

            // Bind texture
            glBindTexture(GL_TEXTURE_2D, fields[fieldIndex]->textureID);

            // Set texture data to field data
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, M, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(textureData.data()));
            // Mipmap generation is mandatory
            glGenerateMipmap(GL_TEXTURE_2D);

            // TODO: This is probably unnecessary, however this will be kept for a bit in case it is necessary.
            // // Set texture storage parameters
            // glTextureStorage2D(fields[fieldIndex]->textureID, 1, GL_RGBA32F, N, M);
            // // Set texture access parameters
            // glBindImageTexture(0, fields[fieldIndex]->textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

            // Clear texture data
            textureData.clear();

            logTrace("Field %d out of %d has been successfully initialised.", fieldIndex + 1, fields.size());
        }

        // Close file
        dataFile.close();

        logDebug("CTDD file path %s successfully loaded.", filePath);

        // Return fields
        return fields;
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());
        return std::vector<std::shared_ptr<Texture2D>>();
    }
}

Texture2D *Texture2D::loadPNG(const char *filePath)
{
    logDebug("Loading texture from PNG file located at path %s", filePath);

    // Load through STB_IMAGE
    int width, height, bpp;
    unsigned char *data = stbi_load(filePath, &width, &height, &bpp, STBI_rgb);
    // Create new texture
    Texture2D *pngTexture = new Texture2D();
    pngTexture->bind();
    pngTexture->setTextureWrap(TextureWrapAxis::UV, TextureWrapMode::CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Free image data
    stbi_image_free(data);

    logDebug("PNG file at path %s successfully loaded.", filePath);
    return pngTexture;
}