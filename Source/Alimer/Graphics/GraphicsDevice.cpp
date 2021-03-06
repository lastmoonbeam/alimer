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

#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/ShaderCompiler.h"
#include "../Math/Math.h"
#include "../Core/Log.h"

#if ALIMER_VULKAN
#include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

#if ALIMER_D3D11
#include "../Graphics/D3D11/D3D11GraphicsDevice.h"
#endif

#if ALIMER_D3D12
#include "../Graphics/D3D12/D3D12GraphicsDevice.h"
#endif

using namespace std;

#if defined(_WIN32)
// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    GraphicsDevice::GraphicsDevice(GraphicsBackend backend, bool validation)
        : _backend(backend)
        , _validation(validation)
    {
        AddSubsystem(this);
    }

    GraphicsDevice* GraphicsDevice::Create(GraphicsBackend preferredGraphicsBackend, bool validation)
    {
        if (preferredGraphicsBackend == GraphicsBackend::Default)
        {
            auto availableDrivers = GraphicsDevice::GetAvailableBackends();

            if (availableDrivers.find(GraphicsBackend::Vulkan) != availableDrivers.end())
            {
                preferredGraphicsBackend = GraphicsBackend::Vulkan;
            }
            else if (availableDrivers.find(GraphicsBackend::Direct3D12) != availableDrivers.end())
            {
                preferredGraphicsBackend = GraphicsBackend::Direct3D12;
            }
            else if (availableDrivers.find(GraphicsBackend::Direct3D11) != availableDrivers.end())
            {
                preferredGraphicsBackend = GraphicsBackend::Direct3D11;
            }
            else
            {
                preferredGraphicsBackend = GraphicsBackend::Empty;
            }
        }

        // Create backend implementation.
        switch (preferredGraphicsBackend)
        {
        case GraphicsBackend::Vulkan:
#if ALIMER_VULKAN
            if (VulkanGraphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Vulkan graphics backend");
                return new VulkanGraphics(validation);
            }
            else
#endif
            {
                ALIMER_LOGWARN("Vulkan graphics backend not supported, fallback to platform default.");
                return Create(GraphicsBackend::Default, validation);
            }

            break;

        case GraphicsBackend::Direct3D12:
#if ALIMER_D3D12
            if (D3D12Graphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Direct3D 12 graphics backend");
                graphics = new D3D12Graphics(validation);
            }
            else
#endif
            {
                ALIMER_LOGWARN("Direct3D 12 graphics backend not supported, fallback to platform default.");
                return Create(GraphicsBackend::Default, validation);
            }

            break;

        case GraphicsBackend::Direct3D11:
#if ALIMER_D3D11
            if (D3D11Graphics::IsSupported())
            {
                ALIMER_LOGINFO("Using Direct3D 11 graphics backend");
                device = new D3D11Graphics(validation);
            }
            else
#endif
            {
                ALIMER_LOGWARN("Direct3D 11 graphics backend not supported, fallback to platform default.");
                return Create(GraphicsBackend::Default, validation);
            }

            break;

        case GraphicsBackend::Empty:
        default:
            ALIMER_LOGWARN("Invalid graphics backend.");
            return nullptr;
        }
    }

    GraphicsDevice::~GraphicsDevice()
    {
        // Destroy undestroyed resources.
        if (_gpuResources.size())
        {
            lock_guard<mutex> lock(_gpuResourceMutex);

            // Release all GPU objects that still exist
            std::sort(_gpuResources.begin(), _gpuResources.end(),
                [](const GpuResource* x, const GpuResource* y)
            {
                return x->GetResourceType() < y->GetResourceType();
            });

            for (size_t i = 0; i < _gpuResources.size(); ++i)
            {
                GpuResource* resource = _gpuResources.at(i);
                ALIMER_ASSERT(resource);
                resource->Destroy();
            }

            _gpuResources.clear();
        }

        RemoveSubsystem(this);
    }

    set<GraphicsBackend> GraphicsDevice::GetAvailableBackends()
    {
        static set<GraphicsBackend> availableBackends;

        if (availableBackends.empty())
        {
            availableBackends.insert(GraphicsBackend::Empty);

#if ALIMER_VULKAN
            if (VulkanGraphics::IsSupported())
            {
                availableBackends.insert(GraphicsBackend::Vulkan);
            }
#endif

#if ALIMER_D3D12
            if (D3D12Graphics::IsSupported())
            {
                availableBackends.insert(GraphicsBackend::Direct3D12);
            }
#endif

#if ALIMER_D3D11
            if (D3D11Graphics::IsSupported())
            {
                availableBackends.insert(GraphicsBackend::Direct3D11);
            }
#endif

        }

        return availableBackends;
        }

    bool GraphicsDevice::Initialize(const RenderingSettings& settings)
    {
        if (_initialized)
        {
            ALIMER_LOGCRITICAL("Cannot Initialize GraphicsDevice if already initialized.");
            return false;
        }

        _settings = settings;
        ALIMER_ASSERT_MSG(settings.windowHandle, "Invalid window handle for graphics creation.");
        _initialized = true;
        return _initialized;
    }

#if TODO
    RenderPass* GraphicsDevice::CreateRenderPass(const RenderPassDescription* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        return CreateRenderPassImpl(descriptor);
    }

    GpuBuffer* GraphicsDevice::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->usage == BufferUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid buffer usage");
        }

        if (!descriptor->size)
        {
            ALIMER_LOGCRITICAL("Cannot create empty buffer");
        }

        if (descriptor->resourceUsage == ResourceUsage::Immutable
            && initialData == nullptr)
        {
            ALIMER_LOGCRITICAL("Immutable Buffer needs valid initial data.");
        }

        return CreateBufferImpl(descriptor, initialData);
    }

    VertexInputFormat* GraphicsDevice::CreateVertexInputFormat(const VertexInputFormatDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        if (!descriptor->attributesCount)
        {
            ALIMER_LOGCRITICAL("Can not define VertexInputFormat with no vertex attributes");
        }

        return CreateVertexInputFormatImpl(descriptor);
    }

    ShaderModule* GraphicsDevice::CreateShaderModule(const std::vector<uint32_t>& spirv)
    {
        if (spirv.empty())
        {
            ALIMER_LOGCRITICAL("Cannot create shader module with empty bytecode");
        }

        return CreateShaderModuleImpl(spirv);
    }

    ShaderModule* GraphicsDevice::CreateShaderModule(const String& file, const String& entryPoint)
    {
        // Load from spirv bytecode.
        vector<uint32_t> spirv;

        /*if (gResources()->Exists(file + ".spv"))
        {
            auto vertexShaderStream = FileSystem::Get().Open(vertexShaderFile + ".spv");
            //auto vertexShaderStream = gResources()->Open(vertexShaderFile + ".spv");
            auto fragmentShaderStream = gResources()->Open(fragmentShaderFile + ".spv");

            // Lookup for SPIR-V compiled shader.
            if (!vertexShaderStream)
            {
                ALIMER_LOGERRORF("GLSL shader does not exists '%s'", vertexShaderFile.c_str());
                return nullptr;
            }

            if (!fragmentShaderStream)
            {
                ALIMER_LOGERRORF("GLSL shader does not exists '%s'", fragmentShaderFile.c_str());
                return nullptr;
            }

            vertexByteCode = vertexShaderStream->ReadBytes();
            fragmentByteCode = fragmentShaderStream->ReadBytes();
        }
        else*/
        {
            // Compile from source GLSL.
            String errorLog;
            if (!ShaderCompiler::Compile(file, entryPoint, spirv, errorLog))
            {
                ALIMER_LOGCRITICALF("Shader compilation failed: %s", errorLog.CString());
            }
        }

        return CreateShaderModule(spirv);
    }

    ShaderProgram* GraphicsDevice::CreateShaderProgram(const ShaderProgramDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        if (!descriptor->stageCount)
        {
            ALIMER_LOGCRITICAL("Cannot create shader program with empty stages");
        }

        for (uint32_t i = 0; i < descriptor->stageCount; i++)
        {
            if (!descriptor->stages[i].module)
            {
                ALIMER_LOGCRITICALF("Cannot create shader program with invalid shader module at index %s", i);
            }

            if (descriptor->stages[i].module->GetStage() == ShaderStage::Compute
                && descriptor->stageCount > 1)
            {
                ALIMER_LOGCRITICAL("Cannot create shader program with compute and graphics stages");
            }
        }

        return CreateShaderProgramImpl(descriptor);
    }

    ShaderProgram* GraphicsDevice::CreateShaderProgram(const ShaderStageDescriptor* stage)
    {
        ALIMER_ASSERT(stage);

        if (stage->module->GetStage() != ShaderStage::Compute)
        {
            ALIMER_LOGCRITICAL("Shader stage module is not compute");
        }

        ShaderProgramDescriptor descriptor = {};
        descriptor.stageCount = 1;
        descriptor.stages = stage;
        return CreateShaderProgramImpl(&descriptor);
    }

    ShaderProgram* GraphicsDevice::CreateShaderProgram(ShaderModule* vertex, ShaderModule* fragment)
    {
        ALIMER_ASSERT(vertex);
        ALIMER_ASSERT(fragment);

        if (vertex->GetStage() != ShaderStage::Vertex)
            ALIMER_LOGCRITICAL("Invalid vertex stage module");

        if (fragment->GetStage() != ShaderStage::Fragment)
            ALIMER_LOGCRITICAL("Invalid fragment stage module");

        std::array<ShaderStageDescriptor, 2> stages;
        stages[0].module = vertex;
        stages[1].module = fragment;
        ShaderProgramDescriptor descriptor = {};
        descriptor.stageCount = 2;
        descriptor.stages = stages.data();
        return CreateShaderProgramImpl(&descriptor);
    }

    Texture* GraphicsDevice::CreateTexture(const TextureDescriptor* descriptor, const ImageLevel* initialData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->usage == TextureUsage::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (descriptor->width == 0
            || descriptor->height == 0
            || descriptor->depth == 0
            || descriptor->arrayLayers == 0
            || descriptor->mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }

        //if (initialData && !(descriptor->usage & TextureUsage::TransferDest))
        //{
        //    ALIMER_LOGCRITICAL("Texture needs the transfer dest usage when creating with initial data.");
        //}

        return CreateTextureImpl(descriptor, initialData);
    }
#endif // TODO

    void GraphicsDevice::AddGpuResource(GpuResource* resource)
    {
        lock_guard<mutex> lock(_gpuResourceMutex);
        _gpuResources.push_back(resource);
    }

    void GraphicsDevice::RemoveGpuResource(GpuResource* resource)
    {
        lock_guard<mutex> lock(_gpuResourceMutex);
        auto it = std::find(_gpuResources.begin(), _gpuResources.end(), resource);
        if (it != _gpuResources.end())
        {
            _gpuResources.erase(it);
        }
    }
    }
