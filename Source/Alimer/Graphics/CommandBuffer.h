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

#pragma once

#include "../Graphics/GpuBuffer.h"
#include "../Graphics/RenderPass.h"
#include "../Graphics/Shader.h"
#include "../Graphics/VertexFormat.h"
#include "../Math/Math.h"
#include "../Math/Color.h"

namespace Alimer
{
    class GraphicsDevice;

    /// Defines a command buffer for recording gpu commands.
    class ALIMER_API CommandBuffer
    {
        friend class GraphicsDevice;

    protected:
        CommandBuffer(GraphicsDevice* graphics);

    public:
        /// Destructor.
        virtual ~CommandBuffer() = default;

        void BeginRenderPass(const RenderPassDescriptor* descriptor);
        void EndRenderPass();

        // Compute
        void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
        void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
        void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
        void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

    protected:
        virtual void BeginRenderPassImpl(const RenderPassDescriptor* descriptor) = 0;
        virtual void EndRenderPassImpl() = 0;

        virtual void DispatchImpl(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

        /*void BeginRenderPass(RenderPass* renderPass, const Color4& clearColor, float clearDepth = 1.0f, uint8_t clearStencil = 0);

        void BeginRenderPass(RenderPass* renderPass,
            const Color4* clearColors, uint32_t numClearColors,
            float clearDepth = 1.0f, uint8_t clearStencil = 0);

        void EndRenderPass();

        virtual void SetViewport(const rect& viewport) = 0;
        virtual void SetScissor(const irect& scissor) = 0;

        void SetShaderProgram(ShaderProgram* program);

        void BindVertexBuffer(GpuBuffer* buffer, uint32_t binding, GpuSize offset = 0, VertexInputRate inputRate = VertexInputRate::Vertex);
        void SetVertexInputFormat(VertexInputFormat* format);
        void BindIndexBuffer(GpuBuffer* buffer, GpuSize offset, IndexType indexType);
        
        void BindBuffer(GpuBuffer* buffer, uint32_t set, uint32_t binding);
        void BindBuffer(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding);
        void BindTexture(Texture* texture, uint32_t set, uint32_t binding);

        // Draw methods
        void Draw(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount = 1u, uint32_t vertexStart = 0u, uint32_t baseInstance = 0u);
        void DrawIndexed(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount = 1u, uint32_t startIndex = 0u);

        //void ExecuteCommands(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers);

    protected:
        virtual void BeginRenderPassCore(RenderPass* renderPass, const Color4* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil) = 0;
        virtual void EndRenderPassCore() = 0;
        //virtual void ExecuteCommandsCore(uint32_t commandBufferCount, CommandBuffer* const* commandBuffers);

        virtual void BindBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding) = 0;
        virtual void BindTextureImpl(Texture* texture, uint32_t set, uint32_t binding) = 0;

        virtual void SetShaderProgramImpl(ShaderProgram* program) = 0;
        virtual void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) = 0;
        virtual void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) = 0;
        virtual void BindVertexBufferImpl(GpuBuffer* buffer, uint32_t binding, uint32_t offset, uint32_t stride, VertexInputRate inputRate) = 0;
        virtual void BindIndexBufferImpl(GpuBuffer* buffer, GpuSize offset, IndexType indexType) = 0;
        virtual void SetVertexInputFormatImpl(VertexInputFormat* format) = 0;
        */

    private:
        /// Graphics subsystem.
        WeakPtr<GraphicsDevice> _graphics;

        inline bool IsInsideRenderPass() const
        {
            return _state == State::InRenderPass;
        }

        enum class State
        {
            None,
            Recording,
            InRenderPass,
            Committed
        };

        State _state = State::None;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(CommandBuffer);
    };
}
