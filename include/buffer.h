#pragma once

#include <intrin.h>

// #include <stdint.h>
#include <vector>

#include <log.h>

enum class BufferElementType
{
    NONE = 0,
    INT,
    INT2,
    INT3,
    INT4,
    UINT,
    UINT2,
    UINT3,
    UINT4,
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    DOUBLE,
    DOUBLE2,
    DOUBLE3,
    DOUBLE4,
    FLOAT_MAT2,
    FLOAT_MAT3,
    FLOAT_MAT4,
    FLOAT_MAT2X3,
    FLOAT_MAT2X4,
    FLOAT_MAT3X2,
    FLOAT_MAT3X4,
    FLOAT_MAT4X2,
    FLOAT_MAT4X3,
    DOUBLE_MAT2,
    DOUBLE_MAT3,
    DOUBLE_MAT4,
    DOUBLE_MAT2X3,
    DOUBLE_MAT2X4,
    DOUBLE_MAT3X2,
    DOUBLE_MAT3X4,
    DOUBLE_MAT4X2,
    DOUBLE_MAT4X3,
};

static uint32_t getBufferElementTypeByteSize(BufferElementType type)
{
    switch (type)
    {
    case BufferElementType::INT:
        return 4;
    case BufferElementType::INT2:
        return 2 * 4;
    case BufferElementType::INT3:
        return 3 * 4;
    case BufferElementType::INT4:
        return 4 * 4;
    case BufferElementType::UINT:
        return 4;
    case BufferElementType::UINT2:
        return 2 * 4;
    case BufferElementType::UINT3:
        return 3 * 4;
    case BufferElementType::UINT4:
        return 4 * 4;
    case BufferElementType::FLOAT:
        return 4;
    case BufferElementType::FLOAT2:
        return 2 * 4;
    case BufferElementType::FLOAT3:
        return 3 * 4;
    case BufferElementType::FLOAT4:
        return 4 * 4;
    case BufferElementType::DOUBLE:
        return 8;
    case BufferElementType::DOUBLE2:
        return 2 * 8;
    case BufferElementType::DOUBLE3:
        return 3 * 8;
    case BufferElementType::DOUBLE4:
        return 4 * 8;
    case BufferElementType::FLOAT_MAT2:
        return 2 * 2 * 4;
    case BufferElementType::FLOAT_MAT3:
        return 3 * 3 * 4;
    case BufferElementType::FLOAT_MAT4:
        return 4 * 4 * 4;
    case BufferElementType::FLOAT_MAT2X3:
        return 2 * 3 * 4;
    case BufferElementType::FLOAT_MAT2X4:
        return 2 * 4 * 4;
    case BufferElementType::FLOAT_MAT3X2:
        return 3 * 2 * 4;
    case BufferElementType::FLOAT_MAT3X4:
        return 3 * 4 * 4;
    case BufferElementType::FLOAT_MAT4X2:
        return 4 * 2 * 4;
    case BufferElementType::FLOAT_MAT4X3:
        return 4 * 3 * 4;
    case BufferElementType::DOUBLE_MAT2:
        return 2 * 2 * 8;
    case BufferElementType::DOUBLE_MAT3:
        return 3 * 3 * 8;
    case BufferElementType::DOUBLE_MAT4:
        return 4 * 4 * 8;
    case BufferElementType::DOUBLE_MAT2X3:
        return 2 * 3 * 8;
    case BufferElementType::DOUBLE_MAT2X4:
        return 2 * 4 * 8;
    case BufferElementType::DOUBLE_MAT3X2:
        return 3 * 2 * 8;
    case BufferElementType::DOUBLE_MAT3X4:
        return 3 * 4 * 8;
    case BufferElementType::DOUBLE_MAT4X2:
        return 4 * 2 * 8;
    case BufferElementType::DOUBLE_MAT4X3:
        return 4 * 3 * 8;
    default:
        logError("Invalid buffer element type!");
        return 0;
    }
}

uint32_t getBufferElementTypeNumberOfRows(BufferElementType type);

struct BufferElement
{
public:
    BufferElementType type;
    uint32_t numComponents;
    uint32_t sizeInBytes;
    bool normalized;

    BufferElement(BufferElementType type, bool normalized)
        : type(type), sizeInBytes(getBufferElementTypeByteSize(type)), normalized(normalized)
    {
        switch (type)
        {
        case BufferElementType::FLOAT:
        case BufferElementType::DOUBLE:
        case BufferElementType::INT:
        case BufferElementType::UINT:
            numComponents = 1;
            break;
        case BufferElementType::FLOAT2:
        case BufferElementType::DOUBLE2:
        case BufferElementType::INT2:
        case BufferElementType::UINT2:
            numComponents = 2;
            break;
        case BufferElementType::FLOAT3:
        case BufferElementType::DOUBLE3:
        case BufferElementType::INT3:
        case BufferElementType::UINT3:
            numComponents = 3;
            break;
        case BufferElementType::FLOAT4:
        case BufferElementType::DOUBLE4:
        case BufferElementType::INT4:
        case BufferElementType::UINT4:
            numComponents = 4;
            break;
        // Matrices only store their number of columns
        case BufferElementType::FLOAT_MAT2:
        case BufferElementType::FLOAT_MAT2X3:
        case BufferElementType::FLOAT_MAT2X4:
        case BufferElementType::DOUBLE_MAT2:
        case BufferElementType::DOUBLE_MAT2X3:
        case BufferElementType::DOUBLE_MAT2X4:
            numComponents = 2;
            break;
        case BufferElementType::FLOAT_MAT3:
        case BufferElementType::FLOAT_MAT3X2:
        case BufferElementType::FLOAT_MAT3X4:
        case BufferElementType::DOUBLE_MAT3:
        case BufferElementType::DOUBLE_MAT3X2:
        case BufferElementType::DOUBLE_MAT3X4:
            numComponents = 3;
            break;
        case BufferElementType::FLOAT_MAT4:
        case BufferElementType::FLOAT_MAT4X2:
        case BufferElementType::FLOAT_MAT4X3:
        case BufferElementType::DOUBLE_MAT4:
        case BufferElementType::DOUBLE_MAT4X2:
        case BufferElementType::DOUBLE_MAT4X3:
            numComponents = 4;
        default:
            logError("Invalid buffer element type!");
            break;
        }
    }
};

struct VertexBufferLayout
{
public:
    VertexBufferLayout(const std::initializer_list<BufferElement> &elements) : m_elements(elements)
    {
        calculateStride();
    }

    inline const std::vector<BufferElement> &getElements() const { return m_elements; };
    inline const uint32_t getStride() const { return m_stride; };

private:
    std::vector<BufferElement> m_elements;
    uint32_t m_stride;

    void calculateStride()
    {
        m_stride = 0;
        for (auto &element : m_elements)
        {
            m_stride += element.sizeInBytes;
        }
    }
};

enum class BufferUsageType
{
    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY,
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,
    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,
};

class VertexBufferSpec
{
public:
    BufferUsageType usage;
};

class VertexBuffer
{
public:
    uint32_t bufferID;
    VertexBufferLayout layout;

    VertexBuffer(void *vertices, uint32_t size, BufferUsageType usageType, VertexBufferLayout layout);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
};

class IndexBuffer
{
public:
    uint32_t bufferID;

    IndexBuffer(const uint32_t *indices, uint32_t count, BufferUsageType usageType);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;
};

class VertexArray
{
public:
    uint32_t arrayID;

    VertexArray();
    ~VertexArray();

    void bindVertexBuffer(VertexBuffer *vertexBuffer);
    void bindIndexBuffer(IndexBuffer *indexBuffer);

    void bind() const;
    void unbind() const;

private:
    uint32_t m_vertexBufferIndex = 0;
};