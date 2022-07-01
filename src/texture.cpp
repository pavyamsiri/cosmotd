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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::~Texture2D()
{
    logTrace("Deleting 2D texture...");
    glDeleteTextures(1, &textureID);
}

void Texture2D::bind(uint32_t target)
{
    glBindTextureUnit(target, textureID);
}

void Texture2D::unbind(uint32_t target)
{
    glBindTextureUnit(target, 0);
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
                    // Alpha channel - current time
                    textureData[(rowIndex * 4 * N) + 4 * columnIndex + 3] = 1.0f;
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
        std::stringstream errorStream;
        errorStream << "Failed to read data from file at path: " << filePath << " - " << e.what();
        logError(errorStream.str().c_str());

        return std::vector<std::shared_ptr<Texture2D>>();
    }
}