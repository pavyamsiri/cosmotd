#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <texture.h>
#include <log.h>

Texture2D::Texture2D()
{
    logTrace("Creating a blank 2D texture...");
    // Generate a texture ID
    glGenTextures(1, &textureID);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

const void Texture2D::saveField(const char *filePath) const
{
    glBindTexture(GL_TEXTURE_2D, this->textureID);
    int M, N;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &M);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &N);

    std::vector<float> textureData(M * N * 4);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, static_cast<void *>(textureData.data()));
    auto errorCode = glGetError();
    glBindTexture(GL_TEXTURE_2D, 0);

    std::ofstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        uint32_t numFields = 1;

        dataFile.open(filePath, std::ios::binary);
        // Write header
        dataFile.write(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));
        dataFile.write(reinterpret_cast<char *>(&M), sizeof(uint32_t));
        dataFile.write(reinterpret_cast<char *>(&N), sizeof(uint32_t));

        // Create list of textures
        std::vector<std::shared_ptr<Texture2D>> fields(numFields);

        // Read data
        for (int fieldIndex = 0; fieldIndex < numFields; fieldIndex++)
        {
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {
                    float fieldValue = textureData[(rowIndex * 4 * N) + 4 * columnIndex + 0];
                    float fieldVelocity = textureData[(rowIndex * 4 * N) + 4 * columnIndex + 1];
                    float fieldAcceleration = textureData[(rowIndex * 4 * N) + 4 * columnIndex + 2];
                    dataFile.write(reinterpret_cast<char *>(&fieldValue), sizeof(float));
                    dataFile.write(reinterpret_cast<char *>(&fieldVelocity), sizeof(float));
                    dataFile.write(reinterpret_cast<char *>(&fieldAcceleration), sizeof(float));
                }
            }
        }

        dataFile.close();
        logTrace("Successfully wrote field data to binary file!");
    }
    catch (std::ifstream::failure &e)
    {
        logError("Failed to open file to write to at path: %s - %s", filePath, e.what());
    }
}

// Texture data setters

std::vector<std::shared_ptr<Texture2D>> Texture2D::loadFromCTDDFile(const char *filePath)
{
    uint32_t numFields;
    uint32_t M;
    uint32_t N;

    std::ifstream dataFile;
    dataFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        dataFile.open(filePath, std::ios::binary);
        // Read header
        dataFile.read(reinterpret_cast<char *>(&numFields), sizeof(uint32_t));
        dataFile.read(reinterpret_cast<char *>(&M), sizeof(uint32_t));
        dataFile.read(reinterpret_cast<char *>(&N), sizeof(uint32_t));

        // Create list of textures
        std::vector<std::shared_ptr<Texture2D>> fields(numFields);

        // Read data
        for (int fieldIndex = 0; fieldIndex < numFields; fieldIndex++)
        {
            std::vector<float> textureData(M * N * 4);
            for (int rowIndex = 0; rowIndex < M; rowIndex++)
            {
                for (int columnIndex = 0; columnIndex < N; columnIndex++)
                {

                    float fieldValue;
                    float fieldVelocity;
                    float fieldAcceleration;

                    dataFile.read(reinterpret_cast<char *>(&fieldValue), sizeof(float));
                    dataFile.read(reinterpret_cast<char *>(&fieldVelocity), sizeof(float));
                    dataFile.read(reinterpret_cast<char *>(&fieldAcceleration), sizeof(float));

                    // Red channel - field value
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 0] = fieldValue;
                    // Green channel - field velocity
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 1] = fieldVelocity;
                    // Blue channel - field accleration
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 2] = fieldAcceleration;
                    // TODO: Having the alpha channel store time is pretty hacky and should be removed. It should just be a
                    // uniform.
                    // Alpha channel - current time
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 3] = 0.1f;
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