#pragma once
// Standard libraries
#include <intrin.h>
#include <vector>

// External libraries

// Internal libraries
#include "log.h"

// The allowed data types for a buffer element.
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

// Helper function that returns the size of each buffer element data type in bytes.
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

// Helper function that returns the number of components for the given buffer element data type.
static uint32_t getBufferElementTypeNumComponents(BufferElementType type)
{
    switch (type)
    {
    case BufferElementType::FLOAT:
    case BufferElementType::DOUBLE:
    case BufferElementType::INT:
    case BufferElementType::UINT:
        return 1;
        break;
    case BufferElementType::FLOAT2:
    case BufferElementType::DOUBLE2:
    case BufferElementType::INT2:
    case BufferElementType::UINT2:
        return 2;
        break;
    case BufferElementType::FLOAT3:
    case BufferElementType::DOUBLE3:
    case BufferElementType::INT3:
    case BufferElementType::UINT3:
        return 3;
        break;
    case BufferElementType::FLOAT4:
    case BufferElementType::DOUBLE4:
    case BufferElementType::INT4:
    case BufferElementType::UINT4:
        return 4;
        break;
    // Matrices only store their number of columns
    case BufferElementType::FLOAT_MAT2:
    case BufferElementType::FLOAT_MAT2X3:
    case BufferElementType::FLOAT_MAT2X4:
    case BufferElementType::DOUBLE_MAT2:
    case BufferElementType::DOUBLE_MAT2X3:
    case BufferElementType::DOUBLE_MAT2X4:
        return 2;
        break;
    case BufferElementType::FLOAT_MAT3:
    case BufferElementType::FLOAT_MAT3X2:
    case BufferElementType::FLOAT_MAT3X4:
    case BufferElementType::DOUBLE_MAT3:
    case BufferElementType::DOUBLE_MAT3X2:
    case BufferElementType::DOUBLE_MAT3X4:
        return 3;
        break;
    case BufferElementType::FLOAT_MAT4:
    case BufferElementType::FLOAT_MAT4X2:
    case BufferElementType::FLOAT_MAT4X3:
    case BufferElementType::DOUBLE_MAT4:
    case BufferElementType::DOUBLE_MAT4X2:
    case BufferElementType::DOUBLE_MAT4X3:
        return 4;
    default:
        logError("Invalid buffer element type!");
        return 0;
    }
}

// Specifies an element in a buffer
struct BufferElement
{
public:
    // Data type of the buffer.
    BufferElementType type;
    // Number of components per element, i.e. INT3 will have 3 components.
    uint32_t numComponents;
    // Size of an element in bytes.
    uint32_t sizeInBytes;
    // Only relevant for float/double data types. If true, each component will be normalised to be in the range [0, 1].
    bool normalized;

    // Constructor
    BufferElement(BufferElementType type, bool normalized)
        : type(type), numComponents(getBufferElementTypeNumComponents(type)),
          sizeInBytes(getBufferElementTypeByteSize(type)), normalized(normalized)
    {
    }
};

// Specifies the layout of a vertex buffer
struct VertexBufferLayout
{
public:
    // Constructor that takes in a list of buffer elements and calculates the stride.
    VertexBufferLayout(const std::initializer_list<BufferElement> &elements) : m_Elements(elements)
    {
        calculateStride();
    }

    // Getter method that returns the vector of buffer elements
    inline const std::vector<BufferElement> &getElements() const { return m_Elements; };
    // Getter method that returns the stride
    inline const uint32_t getStride() const { return m_Stride; };

private:
    // List of buffer elements associated with the vertex buffer.
    std::vector<BufferElement> m_Elements;
    // The tatal size of the vertex buffer in bytes.
    uint32_t m_Stride;

    // Calculates the stride from the list of buffer elements associated with the vertex buffer.
    void calculateStride()
    {
        m_Stride = 0;
        for (auto &element : m_Elements)
        {
            m_Stride += element.sizeInBytes;
        }
    }
};

// The usage types of a buffer.
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

// Wraps a OpenGL vertex buffer.
class VertexBuffer
{
public:
    //  OpenGL buffer ID
    uint32_t bufferID = 0;
    //  Data layout of the vertex buffer.
    VertexBufferLayout layout;

    // Constructor that takes in vertex data
    VertexBuffer(void *vertices, uint32_t size, BufferUsageType usageType, VertexBufferLayout layout);
    // Destructor
    ~VertexBuffer();

    // Binds the vertex buffer
    void bind();
    // Unbinds the vertex buffer
    void unbind();
};

// Wraps a OpenGL index buffer.
class IndexBuffer
{
public:
    // OpenGL buffer ID
    uint32_t bufferID = 0;

    // Constructor that takes in a list of indices
    IndexBuffer(const uint32_t *indices, uint32_t count, BufferUsageType usageType);
    // Destructor
    ~IndexBuffer();

    // Binds the index buffer
    void bind();
    // Unbinds the index buffer
    void unbind();
};

// Wraps a OpenGL VAO
class VertexArray
{
public:
    // OpenGL vertex array object ID
    uint32_t arrayID = 0;

    // Constructor
    VertexArray();
    // Destructor
    ~VertexArray();

    // Bind vertex buffer to VAO
    void bindVertexBuffer(VertexBuffer *vertexBuffer);
    // Bind index buffer to VAO
    void bindIndexBuffer(IndexBuffer *indexBuffer);

    // Bind the VAO
    void bind();
    // Unbind the VAO
    void unbind();

private:
    // Index that tracks the current vertex buffer index
    uint32_t m_VertexBufferIndex = 0;
};