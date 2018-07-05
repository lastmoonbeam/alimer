//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Graphics/CommandBuffer.h"
#include "Graphics/Graphics.h"
#include "../Core/Log.h"

namespace Alimer
{
    CommandBuffer::CommandBuffer(Graphics* graphics)
        : GpuResource(graphics, GpuResourceType::CommandBuffer)
        , _hasPendingEncoder(false)
    {
    }

    void CommandBuffer::Commit()
    {
        if (_hasPendingEncoder)
        {
            ALIMER_LOGCRITICAL("Cannot commit with pending command encoders.");
        }

        CommitCore();
    }

    void CommandBuffer::EndEncoderEncoding()
    {
        _hasPendingEncoder = false;
    }

    RenderPassCommandEncoder* CommandBuffer::CreateRenderPassCommandEncoder(RenderPass* renderPass, const Color& clearColor, float clearDepth, uint8_t clearStencil)
    {
        if (_hasPendingEncoder)
        {
            ALIMER_LOGCRITICAL("Cannot create RenderPassCommandEncoder while other encoder is active");
        }

        _hasPendingEncoder = true;
        return CreateRenderPassCommandEncoderCore(renderPass, &clearColor, 1, clearDepth, clearStencil);
    }

    RenderPassCommandEncoder* CommandBuffer::CreateRenderPassCommandEncoder(RenderPass* renderPass,
        const Color* clearColors, uint32_t numClearColors,
        float clearDepth, uint8_t clearStencil)
    {
        if (_hasPendingEncoder)
        {
            ALIMER_LOGCRITICAL("Cannot create RenderPassCommandEncoder while other encoder is active");
        }

        _hasPendingEncoder = true;
        return CreateRenderPassCommandEncoderCore(renderPass, clearColors, numClearColors, clearDepth, clearStencil);
    }

    void CommandBuffer::SetVertexBuffer(GpuBuffer* buffer, uint32_t binding, uint64_t offset, VertexInputRate inputRate)
    {
        ALIMER_ASSERT(binding < MaxVertexBufferBindings);
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetBufferUsage() & BufferUsage::Vertex);

        if (_vbo.buffers[binding] != buffer
            || _vbo.offsets[binding] != offset)
        {
            _dirtyVbos |= 1u << binding;
        }

        uint64_t stride = buffer->GetElementSize();
        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            SetDirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        _vbo.buffers[binding] = buffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
        OnSetVertexBuffer(buffer, binding, offset);
    }

    void CommandBuffer::SetIndexBuffer(GpuBuffer* buffer, uint32_t offset, IndexType indexType)
    {
        ALIMER_ASSERT(buffer);
        ALIMER_ASSERT(buffer->GetBufferUsage() & BufferUsage::Index);

        SetIndexBufferCore(buffer, offset, indexType);
    }

    void CommandBuffer::SetUniformBuffer(uint32_t set, uint32_t binding, const GpuBuffer* buffer)
    {
        ALIMER_ASSERT(set < MaxDescriptorSets);
        ALIMER_ASSERT(binding < MaxBindingsPerSet);
        ALIMER_ASSERT(buffer->GetBufferUsage() & BufferUsage::Uniform);
        auto &b = _bindings.bindings[set][binding];

        uint64_t range = buffer->GetSize();
        if (buffer == b.buffer.buffer
            && b.buffer.offset == 0
            && b.buffer.range == range)
            return;

        b.buffer = { buffer, 0, range };
        _dirtySets |= 1u << set;
    }

    

    void CommandBuffer::DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        DrawIndexedCore(topology, indexCount, instanceCount, startIndex);
    }
}
