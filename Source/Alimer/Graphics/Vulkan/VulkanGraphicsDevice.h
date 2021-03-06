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

#include "../GraphicsDevice.h"
#include "../../Base/HashMap.h"
#include "VulkanRenderPass.h"
#include <queue>

#ifdef ALIMER_VULKAN_MT
#   include <atomic>
#   include <mutex>
#   include <condition_variable>
#endif

namespace Alimer
{
    class VulkanSwapchain;
    class VulkanDescriptorSetAllocator;
    class VulkanPipelineLayout;

    /// Vulkan graphics backend.
    class VulkanGraphics final : public GraphicsDevice
    {
    public:
        static bool IsSupported();

        /// Construct. Set parent shader and defines but do not compile yet.
        VulkanGraphics(bool validation);
        /// Destruct.
        ~VulkanGraphics() override;

        void NotifyFalidationError(const char* message);

        bool Initialize(const RenderingSettings& settings) override;
        bool WaitIdle() override;

        bool BeginFrame() override;
        void EndFrame() override;

        /*
        RenderPass* CreateRenderPassImpl(const RenderPassDescription* descriptor) override;
        GpuBuffer* CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData) override;
        VertexInputFormat* CreateVertexInputFormatImpl(const VertexInputFormatDescriptor* descriptor) override;
        ShaderModule* CreateShaderModuleImpl(const std::vector<uint32_t>& spirv) override;
        ShaderProgram* CreateShaderProgramImpl(const ShaderProgramDescriptor* descriptor) override;

        Texture* CreateTextureImpl(const TextureDescriptor* descriptor, const ImageLevel* initialData) override;
        */

        VkInstance GetInstance() const { return _instance; }
        VkPhysicalDevice GetPhysicalDevice() const { return _physicalDevice; }
        VkDevice GetDevice() const { return _device; }
        VmaAllocator GetVmaAllocator() const { return _allocator; }
        VkPipelineCache GetPipelineCache() const { return _pipelineCache; }

        uint32_t GetGraphicsQueueFamily() const { return _graphicsQueueFamily; }
        uint32_t GetComputeQueueFamily() const { return _computeQueueFamily; }
        uint32_t GetTransferQueueFamily() const { return _transferQueueFamily; }

        uint64_t AllocateCookie();

        VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags);
        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
        void ClearImageWithColor(VkCommandBuffer commandBuffer, VkImage image, VkImageSubresourceRange range, VkImageAspectFlags aspect, VkImageLayout sourceLayout, VkImageLayout destLayout, VkAccessFlagBits srcAccessMask, VkClearColorValue *clearValue);

        const VulkanFramebuffer& RequestFramebuffer(const RenderPassDescriptor* descriptor);
        const VulkanRenderPass& RequestRenderPass(const RenderPassDescriptor* descriptor);

        //VulkanDescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout &layout);
        //VulkanPipelineLayout* RequestPipelineLayout(const ResourceLayout &layout);

        
        VkSurfaceKHR CreateSurface(const RenderingSettings& settings);

        // Fence
        VkFence AcquireFence();
        void ReleaseFence(VkFence fence);

        // Semaphore
        VkSemaphore AcquireSemaphore();
        void ReleaseSemaphore(VkSemaphore semaphore);

    private:
        void createMemoryAllocator();

        bool _supportsExternal = false;
        bool _supportsDedicated = false;
        bool _supportsImageFormatList = false;
        bool _supportsDebugMarker = false;
        bool _supportsDebugUtils = false;

        VkInstance _instance = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT _debugCallback = VK_NULL_HANDLE;

        // PhysicalDevice
        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties _deviceProperties;
        VkPhysicalDeviceMemoryProperties _deviceMemoryProperties;
        VkPhysicalDeviceFeatures _deviceFeatures;
        std::vector<VkQueueFamilyProperties> _queueFamilyProperties;

        // LogicalDevice
        VkDevice _device = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;

        // Queue's.
        VkQueue _graphicsQueue = VK_NULL_HANDLE;
        VkQueue _computeQueue = VK_NULL_HANDLE;
        VkQueue _transferQueue = VK_NULL_HANDLE;
        uint32_t _graphicsQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _computeQueueFamily = VK_QUEUE_FAMILY_IGNORED;
        uint32_t _transferQueueFamily = VK_QUEUE_FAMILY_IGNORED;

        // Main swap chain.
        UniquePtr<VulkanSwapchain> _mainSwapchain;

        // Pipeline cache.
        VkPipelineCache _pipelineCache = VK_NULL_HANDLE;

        // Default command pool.
        VkCommandPool _commandPool = VK_NULL_HANDLE;

        uint32_t _swapchainImageIndex = 0;
        VkSemaphore _swapchainImageAcquiredSemaphore = VK_NULL_HANDLE;

        // Fence pool.
        std::mutex _fenceLock;
        std::set<VkFence> _allFences;
        std::queue<VkFence> _availableFences;

        // Semaphore pool
        std::mutex _semaphoreLock;
        std::set<VkSemaphore> _allSemaphores;
        std::queue<VkSemaphore> _availableSemaphores;

#ifdef ALIMER_VULKAN_MT
        std::atomic<uint64_t> _cookie;
#else
        uint64_t _cookie = 0;
#endif

        template <typename T>
        class Cache
        {
        public:
            T* Find(Util::Hash hash) const
            {
                auto itr = _hashMap.find(hash);
                auto *ret = itr != _hashMap.end() ? itr->second.get() : nullptr;
                return ret;
            }

            T* Insert(Util::Hash hash, std::unique_ptr<T> value)
            {
                auto &cache = _hashMap[hash];
                if (!cache)
                    cache = std::move(value);

                auto *ret = cache.get();
                return ret;
            }

            void Clear()
            {
                _hashMap.clear();
            }

            Util::HashMap<std::unique_ptr<T>> &GetHashMap()
            {
                return _hashMap;
            }

            const Util::HashMap<std::unique_ptr<T>>& GetHashMap() const
            {
                return _hashMap;
            }

        private:
            Util::HashMap<std::unique_ptr<T>> _hashMap;
        };

        // Cache
        VkFramebufferAllocator _framebufferAllocator;
        Cache<VulkanRenderPass> _renderPasses;

        //HashMap<std::unique_ptr<VulkanDescriptorSetAllocator>> _descriptorSetAllocators;
        //HashMap<std::unique_ptr<VulkanPipelineLayout>> _pipelineLayouts;
    };
}
