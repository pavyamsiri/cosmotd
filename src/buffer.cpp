#include <glad/glad.h>
#include <intrin.h>

#include <buffer.h>
#include <log.h>
#include <sstream>

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
    default:
        logFatal("Invalid buffer usage type!");
        // exit(-1);
        return 0;
    }
}

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
        logFatal("Invalid buffer element type!");
        __debugbreak();
        return 0;
    }
}

VertexBuffer::VertexBuffer(void *vertices, uint32_t size, BufferUsageType usageType, VertexBufferLayout layout) : layout(layout)
{
    logTrace("Vertex buffer created!.");
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, convertBufferUsageTypeToOpenGLEnum(usageType));
}

VertexBuffer::~VertexBuffer()
{
    logTrace("Vertex buffer deleted.");
    glDeleteBuffers(1, &bufferID);
}

void VertexBuffer::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
}

void VertexBuffer::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(uint32_t *indices, uint32_t count, BufferUsageType usageType)
{
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, convertBufferUsageTypeToOpenGLEnum(usageType));
}

IndexBuffer::~IndexBuffer()
{
    logTrace("Index buffer deleted.");
    glDeleteBuffers(1, &bufferID);
}

void IndexBuffer::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
}

void IndexBuffer::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &arrayID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &arrayID);
}

void VertexArray::bindVertexBuffer(VertexBuffer *vertexBuffer)
{
    glBindVertexArray(arrayID);
    vertexBuffer->bind();

    const VertexBufferLayout layout = vertexBuffer->layout;
    uint32_t currentOffset = 0;
    uint32_t numCols;
    uint32_t numRows;

    for (const auto &element : layout.getElements())
    {
        switch (element.type)
        {
        case BufferElementType::FLOAT:
        case BufferElementType::FLOAT2:
        case BufferElementType::FLOAT3:
        case BufferElementType::FLOAT4:
            glEnableVertexAttribArray(m_vertexBufferIndex);
            glVertexAttribPointer(
                m_vertexBufferIndex,
                element.numComponents,
                GL_FLOAT,
                element.normalized ? GL_TRUE : GL_FALSE,
                layout.getStride(),
                (const void *)currentOffset);
            m_vertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::DOUBLE:
        case BufferElementType::DOUBLE2:
        case BufferElementType::DOUBLE3:
        case BufferElementType::DOUBLE4:
            glEnableVertexAttribArray(m_vertexBufferIndex);
            glVertexAttribLPointer(
                m_vertexBufferIndex,
                element.numComponents,
                GL_DOUBLE,
                layout.getStride(),
                (const void *)currentOffset);
            m_vertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::INT:
        case BufferElementType::INT2:
        case BufferElementType::INT3:
        case BufferElementType::INT4:
            glEnableVertexAttribArray(m_vertexBufferIndex);
            glVertexAttribIPointer(
                m_vertexBufferIndex,
                element.numComponents,
                GL_INT,
                layout.getStride(),
                (const void *)currentOffset);
            m_vertexBufferIndex++;
            currentOffset = currentOffset + element.sizeInBytes;
            break;
        case BufferElementType::UINT:
        case BufferElementType::UINT2:
        case BufferElementType::UINT3:
        case BufferElementType::UINT4:
            glEnableVertexAttribArray(m_vertexBufferIndex);
            glVertexAttribIPointer(
                m_vertexBufferIndex,
                element.numComponents,
                GL_UNSIGNED_INT,
                layout.getStride(),
                (const void *)currentOffset);
            m_vertexBufferIndex++;
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
                glEnableVertexAttribArray(m_vertexBufferIndex);
                glVertexAttribPointer(
                    m_vertexBufferIndex,
                    numRows,
                    GL_FLOAT,
                    element.normalized ? GL_TRUE : GL_FALSE,
                    layout.getStride(),
                    (const void *)currentOffset);
                // NOTE: I think this is used to skip ahead one attribute when indexing in order to group all matrix columns
                // together.
                glVertexAttribDivisor(m_vertexBufferIndex, 1);
                m_vertexBufferIndex++;
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
                glEnableVertexAttribArray(m_vertexBufferIndex);
                glVertexAttribLPointer(
                    m_vertexBufferIndex,
                    numRows,
                    GL_DOUBLE,
                    layout.getStride(),
                    (const void *)currentOffset);
                glVertexAttribDivisor(m_vertexBufferIndex, 1);
                m_vertexBufferIndex++;
                // Offset by 8 bytes (double byte size) * number of rows
                currentOffset = currentOffset + (numRows * 8);
            }
            break;
        default:
            logFatal("Invalid buffer element type!");
            __debugbreak();
            break;
        }
    }
}

void VertexArray::bindIndexBuffer(IndexBuffer *indexBuffer)
{
    glBindVertexArray(arrayID);
    indexBuffer->bind();
}

void VertexArray::bind() const
{
    glBindVertexArray(arrayID);
}
void VertexArray::unbind() const
{
    glBindVertexArray(0);
}