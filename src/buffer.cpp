// Standard libraries
#include <intrin.h>

// External libraries
#include <glad/glad.h>
#include <sstream>

// Internal libraries
#include "buffer.h"
#include "log.h"

// Helper function to convert BufferUsageType to its corresponding GLenum.
GLenum convertBufferUsageTypeToOpenGLEnum(BufferUsageType type)
{
    switch (type)
    {
    case BufferUsageType::STREAM_DRAW:
        return GL_STREAM_DRAW;
    case BufferUsageType::STREAM_READ:
        return GL_STREAM_READ;
    case BufferUsageType::STREAM_COPY:
        return GL_STREAM_COPY;
    case BufferUsageType::STATIC_DRAW:
        return GL_STATIC_DRAW;
    case BufferUsageType::STATIC_READ:
        return GL_STATIC_READ;
    case BufferUsageType::STATIC_COPY:
        return GL_STATIC_COPY;
    case BufferUsageType::DYNAMIC_DRAW:
        return GL_DYNAMIC_DRAW;
    case BufferUsageType::DYNAMIC_READ:
        return GL_DYNAMIC_READ;
    case BufferUsageType::DYNAMIC_COPY:
        return GL_DYNAMIC_COPY;
    }
    logError("Invalid buffer usage type!");
    return 0;
}

// Helper function to return the number of rows of BufferUsageType. This is primarily for use with matrices.
uint32_t getBufferElementTypeNumberOfRows(BufferElementType type)
{
    switch (type)
    {
    case BufferElementType::FLOAT:
    case BufferElementType::FLOAT2:
    case BufferElementType::FLOAT3:
    case BufferElementType::FLOAT4:
    case BufferElementType::DOUBLE:
    case BufferElementType::DOUBLE2:
    case BufferElementType::DOUBLE3:
    case BufferElementType::DOUBLE4:
    case BufferElementType::INT:
    case BufferElementType::INT2:
    case BufferElementType::INT3:
    case BufferElementType::INT4:
    case BufferElementType::UINT:
    case BufferElementType::UINT2:
    case BufferElementType::UINT3:
    case BufferElementType::UINT4:
        return 1;
    case BufferElementType::FLOAT_MAT2:
    case BufferElementType::FLOAT_MAT3X2:
    case BufferElementType::FLOAT_MAT4X2:
    case BufferElementType::DOUBLE_MAT2:
    case BufferElementType::DOUBLE_MAT3X2:
    case BufferElementType::DOUBLE_MAT4X2:
        return 2;
    case BufferElementType::FLOAT_MAT3:
    case BufferElementType::FLOAT_MAT2X3:
    case BufferElementType::FLOAT_MAT4X3:
    case BufferElementType::DOUBLE_MAT3:
    case BufferElementType::DOUBLE_MAT2X3:
    case BufferElementType::DOUBLE_MAT4X3:
        return 3;
    case BufferElementType::FLOAT_MAT4:
    case BufferElementType::FLOAT_MAT2X4:
    case BufferElementType::FLOAT_MAT3X4:
    case BufferElementType::DOUBLE_MAT4:
    case BufferElementType::DOUBLE_MAT2X4:
    case BufferElementType::DOUBLE_MAT3X4:
        return 4;
    default:
        logError("Invalid buffer element type!");
        return 0;
    }
}

VertexBuffer::VertexBuffer(void *vertices, uint32_t size, BufferUsageType usageType, VertexBufferLayout layout) : layout(layout)
{
    logDebug("Vertex buffer is begin created...");

    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, convertBufferUsageTypeToOpenGLEnum(usageType));

    logDebug("Vertex buffer successfully created with ID %d.", bufferID);
}

VertexBuffer::~VertexBuffer()
{
    logDebug("Vertex buffer with ID %d is being destroyed...", bufferID);
    glDeleteBuffers(1, &bufferID);
    logDebug("Vertex buffer with ID %d has been destroyed.", bufferID);
}

void VertexBuffer::bind()
{
    logTrace("Binding vertex buffer with ID %d.", bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
}

void VertexBuffer::unbind()
{
    logTrace("Unbinding vertex buffer with ID %d.", bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(const uint32_t *indices, uint32_t count, BufferUsageType usageType)
{
    logDebug("Index buffer is begin created...");
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, convertBufferUsageTypeToOpenGLEnum(usageType));
    logDebug("Index buffer successfully created with ID %d.", bufferID);
}

IndexBuffer::~IndexBuffer()
{
    logDebug("Index buffer with ID %d is being destroyed...", bufferID);
    glDeleteBuffers(1, &bufferID);
    logDebug("Index buffer with ID %d has been destroyed.", bufferID);
}

void IndexBuffer::bind()
{
    logTrace("Binding index buffer with ID %d.", bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
}

void IndexBuffer::unbind()
{
    logTrace("Unbinding index buffer with ID %d.", bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
}

VertexArray::VertexArray()
{
    logDebug("Vertex array is being created...");
    glGenVertexArrays(1, &arrayID);
    logDebug("Vertex array successfully created with ID %d.", arrayID);
}

VertexArray::~VertexArray()
{
    logTrace("Vertex array with ID %d is being destroyed...", arrayID);
    glDeleteVertexArrays(1, &arrayID);
    logTrace("Vertex array with ID %d has been destroyed...", arrayID);
}

void VertexArray::bindVertexBuffer(VertexBuffer *vertexBuffer)
{
    logTrace("Binding vertex buffer with ID %d to vertex array with ID %d...", vertexBuffer->bufferID, arrayID);
    // Bind the VAO and vertex buffer
    glBindVertexArray(arrayID);
    vertexBuffer->bind();

    VertexBufferLayout layout = vertexBuffer->layout;
    uint32_t currentOffset = 0;
    uint32_t numCols;
    uint32_t numRows;

    // Iterate through the vertex buffer elements
    for (const auto &element : layout.getElements())
    {
        // Set the vertex attributes depending on the buffer element specification.
        switch (element.type)
        {
        case BufferElementType::FLOAT:
        case BufferElementType::FLOAT2:
        case BufferElementType::FLOAT3:
        case BufferElementType::FLOAT4:
            glEnableVertexAttribArray(m_VertexBufferIndex);
            glVertexAttribPointer(
                m_VertexBufferIndex,
                element.numComponents,
                GL_FLOAT,
                element.normalized ? GL_TRUE : GL_FALSE,
                layout.getStride(),
                (const void *)currentOffset);
            m_VertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::DOUBLE:
        case BufferElementType::DOUBLE2:
        case BufferElementType::DOUBLE3:
        case BufferElementType::DOUBLE4:
            glEnableVertexAttribArray(m_VertexBufferIndex);
            glVertexAttribLPointer(
                m_VertexBufferIndex,
                element.numComponents,
                GL_DOUBLE,
                layout.getStride(),
                (const void *)currentOffset);
            m_VertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::INT:
        case BufferElementType::INT2:
        case BufferElementType::INT3:
        case BufferElementType::INT4:
            glEnableVertexAttribArray(m_VertexBufferIndex);
            glVertexAttribIPointer(
                m_VertexBufferIndex,
                element.numComponents,
                GL_INT,
                layout.getStride(),
                (const void *)currentOffset);
            m_VertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::UINT:
        case BufferElementType::UINT2:
        case BufferElementType::UINT3:
        case BufferElementType::UINT4:
            glEnableVertexAttribArray(m_VertexBufferIndex);
            glVertexAttribIPointer(
                m_VertexBufferIndex,
                element.numComponents,
                GL_UNSIGNED_INT,
                layout.getStride(),
                (const void *)currentOffset);
            m_VertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::FLOAT_MAT2:
        case BufferElementType::FLOAT_MAT3:
        case BufferElementType::FLOAT_MAT4:
        case BufferElementType::FLOAT_MAT2X3:
        case BufferElementType::FLOAT_MAT2X4:
        case BufferElementType::FLOAT_MAT3X2:
        case BufferElementType::FLOAT_MAT3X4:
        case BufferElementType::FLOAT_MAT4X2:
        case BufferElementType::FLOAT_MAT4X3:
            numCols = element.numComponents;
            numRows = getBufferElementTypeNumberOfRows(element.type);
            // GLM matrices are column major
            for (int i = 0; i < numCols; i++)
            {
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribPointer(
                    m_VertexBufferIndex,
                    numRows,
                    GL_FLOAT,
                    element.normalized ? GL_TRUE : GL_FALSE,
                    layout.getStride(),
                    (const void *)currentOffset);
                // NOTE: I think this is used to skip ahead one attribute when indexing in order to group all matrix columns
                // together.
                glVertexAttribDivisor(m_VertexBufferIndex, 1);
                m_VertexBufferIndex++;
                // Offset by 4 bytes (float byte size) * number of rows
                currentOffset = currentOffset + (numRows * 4);
            }
            break;
        // Same algorithm as float matrices just with double as the data type
        case BufferElementType::DOUBLE_MAT2:
        case BufferElementType::DOUBLE_MAT3:
        case BufferElementType::DOUBLE_MAT4:
        case BufferElementType::DOUBLE_MAT2X3:
        case BufferElementType::DOUBLE_MAT2X4:
        case BufferElementType::DOUBLE_MAT3X2:
        case BufferElementType::DOUBLE_MAT3X4:
        case BufferElementType::DOUBLE_MAT4X2:
        case BufferElementType::DOUBLE_MAT4X3:
            numCols = element.numComponents;
            numRows = getBufferElementTypeNumberOfRows(element.type);
            for (int i = 0; i < numCols; i++)
            {
                glEnableVertexAttribArray(m_VertexBufferIndex);
                glVertexAttribLPointer(
                    m_VertexBufferIndex,
                    numRows,
                    GL_DOUBLE,
                    layout.getStride(),
                    (const void *)currentOffset);
                glVertexAttribDivisor(m_VertexBufferIndex, 1);
                m_VertexBufferIndex++;
                // Offset by 8 bytes (double byte size) * number of rows
                currentOffset = currentOffset + (numRows * 8);
            }
            break;
        default:
            logError("Invalid buffer element type!");
            break;
        }
    }
    logTrace("Vertex buffer with ID %d successfully bound to vertex array with ID %d.", vertexBuffer->bufferID, arrayID);
}

void VertexArray::bindIndexBuffer(IndexBuffer *indexBuffer)
{
    logTrace("Binding index buffer with ID %d bound to vertex array with ID %d.", indexBuffer->bufferID, arrayID);
    glBindVertexArray(arrayID);
    indexBuffer->bind();
    logTrace("Index buffer with ID %d successfully bound to vertex array with ID %d.", indexBuffer->bufferID, arrayID);
}

void VertexArray::bind()
{
    logTrace("Binding vertex array with ID %d...", arrayID);
    glBindVertexArray(arrayID);
}
void VertexArray::unbind()
{
    logTrace("Unbinding vertex array with ID %d...", arrayID);
    glBindVertexArray(0);
}