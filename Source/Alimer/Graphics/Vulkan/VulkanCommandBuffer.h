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

#include "../CommandBuffer.h"
#include "VulkanPrerequisites.h"
#include <vector>

namespace Alimer
{
	class VulkanGraphics;

	/// Vulkan CommandBuffer.
	class VulkanCommandBuffer final : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanGraphics* graphics, VkCommandPool commandPool, VkCommandBuffer vkCommandBuffer);
		~VulkanCommandBuffer() override;

		void Begin();
		void End();
		uint64_t Commit(bool waitForCompletion) override;

		void BeginRenderPass(const RenderPassDescriptor& descriptor) override;
		void EndRenderPass() override;

		void SetPipeline(const PipelineStatePtr& pipeline) override;

		void DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance) override;
		void DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex) override;

		inline VkCommandBuffer GetVkCommandBuffer() const { return _vkCommandBuffer; }

	private:
		VkDevice _logicalDevice;
		VkCommandPool _commandPool;
		VkCommandBuffer _vkCommandBuffer;
		uint64_t _fenceValue{};
	};
}