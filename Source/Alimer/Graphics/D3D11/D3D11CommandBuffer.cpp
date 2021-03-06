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

#include "D3D11CommandBuffer.h"
#include "D3D11GraphicsDevice.h"
#include "D3D11Texture.h"
#include "D3D11RenderPass.h"
#include "D3D11Shader.h"
#include "D3D11GpuBuffer.h"
#include "D3D11VertexInputFormat.h"
#include "../D3D/D3DConvert.h"
#include "../../Graphics/VertexFormat.h"
#include "../../Math/MathUtil.h"
#include "../../Core/Log.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    static ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};

    // D3D11CommandContext
    D3D11CommandBuffer::D3D11CommandBuffer(D3D11Graphics* graphics, ID3D11DeviceContext1* context)
        : _graphics(graphics)
        , _context(context)
        , _immediate(context->GetType() == D3D11_DEVICE_CONTEXT_IMMEDIATE)
    {
        HRESULT hr = S_OK;

        if (context->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED)
        {
            D3D11_FEATURE_DATA_THREADING threadingCaps = { FALSE, FALSE };

            hr = graphics->GetD3DDevice()->CheckFeatureSupport(D3D11_FEATURE_THREADING, &threadingCaps, sizeof(threadingCaps));
            if (SUCCEEDED(hr) && !threadingCaps.DriverCommandLists)
            {
                // The runtime emulates command lists.
                _needWorkaround = true; 
            }
        }

        Reset();
    }

    D3D11CommandBuffer::~D3D11CommandBuffer()
    {
        if (!_immediate)
        {
            SafeRelease(_context);
        }
    }

    void D3D11CommandBuffer::Reset()
    {
        _currentRenderPass = nullptr;
        _currentColorAttachmentsBound = 0;
        _currentTopology = PrimitiveTopology::Count;
        _currentShader = nullptr;
        _currentRasterizerState = nullptr;
        _currentDepthStencilState = nullptr;
        _currentBlendState = nullptr;
        _currentVertexInputFormat = nullptr;

        _currentInputLayout.first = 0;
        _currentInputLayout.second = 0;

        // _dirtySets= ~0u;
        // _dirtyVbos = ~0u;
        memset(_vbo.buffers, 0, sizeof(_vbo.buffers));
        memset(_vbo.d3dBuffers, 0, sizeof(_vbo.d3dBuffers));
        memset(&_bindings, 0, sizeof(_bindings));
        _inputLayoutDirty = false;
    }

    bool D3D11CommandBuffer::BeginCore()
    {
        if (!_immediate)
        {
            Reset();
        }

        return true;
    }

    bool D3D11CommandBuffer::EndCore()
    {
        return true;
    }

    void D3D11CommandBuffer::BeginRenderPassCore(RenderPass* renderPass, const Color4* clearColors, uint32_t numClearColors, float clearDepth, uint8_t clearStencil)
    {
        if (!renderPass)
        {
            renderPass = _graphics->GetBackbufferRenderPass();
        }

        _currentRenderPass = static_cast<D3D11RenderPass*>(renderPass);
        _currentRenderPass->Bind(_context);
        _currentColorAttachmentsBound = renderPass->GetColorAttachmentsCount();

        for (uint32_t i = 0; i < _currentColorAttachmentsBound; ++i)
        {
            const RenderPassAttachment& colorAttachment = renderPass->GetColorAttachment(i);

            switch (colorAttachment.loadAction)
            {
            case LoadAction::Clear:
                _context->ClearRenderTargetView(_currentRenderPass->GetRTV(i), &clearColors[i].r);
                break;

            default:
                break;
            }
        }

        // Set viewport and scissor from fbo.
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,
            float(renderPass->GetWidth()), float(renderPass->GetHeight()),
            0.0f, 1.0f
        };

        D3D11_RECT scissor = { 0, 0,
            LONG(renderPass->GetWidth()), LONG(renderPass->GetHeight())
        };

        _context->RSSetViewports(1, &viewport);
        _context->RSSetScissorRects(1, &scissor);
    }

    void D3D11CommandBuffer::EndRenderPassCore()
    {
        _context->OMSetRenderTargets(_currentColorAttachmentsBound, nullViews, nullptr);
        _currentRenderPass = nullptr;
    }

    void D3D11CommandBuffer::SetViewport(const rect& viewport)
    {
        D3D11_VIEWPORT d3dViewport = {
            viewport.x, viewport.y,
            viewport.width, viewport.height,
            D3D11_MIN_DEPTH, D3D11_MAX_DEPTH
        };
        _context->RSSetViewports(1, &d3dViewport);
    }

    void D3D11CommandBuffer::SetScissor(const irect& scissor)
    {
        D3D11_RECT scissorD3D;
        scissorD3D.left = scissor.x;
        scissorD3D.top = scissor.y;
        scissorD3D.right = scissor.x + scissor.width;
        scissorD3D.bottom = scissor.y + scissor.height;
        _context->RSSetScissorRects(1, &scissorD3D);
    }

    void D3D11CommandBuffer::SetShaderProgramImpl(ShaderProgram* program)
    {
        if (_currentShader != program)
        {
            _currentShader = static_cast<D3D11Shader*>(program);
            _inputLayoutDirty = true;
        }
    }

    void D3D11CommandBuffer::BindVertexBufferImpl(GpuBuffer* buffer, uint32_t binding, uint32_t offset, uint32_t stride, VertexInputRate inputRate)
    {
        if (_vbo.buffers[binding] != buffer
            || _vbo.offsets[binding] != offset)
        {
            _vbo.d3dBuffers[binding] = static_cast<D3D11Buffer*>(buffer)->GetHandle();

            _dirtyVbos |= 1u << binding;
        }

        if (_vbo.strides[binding] != stride
            || _vbo.inputRates[binding] != inputRate)
        {
            //set_dirty(COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT);
        }

        _vbo.buffers[binding] = buffer;
        _vbo.offsets[binding] = offset;
        _vbo.strides[binding] = stride;
        _vbo.inputRates[binding] = inputRate;
    }

    void D3D11CommandBuffer::SetVertexInputFormatImpl(VertexInputFormat* format)
    {
        if (_currentVertexInputFormat != format)
        {
            _currentVertexInputFormat = static_cast<D3D11VertexInputFormat*>(format);
            _inputLayoutDirty = true;
        }
    }

    void D3D11CommandBuffer::BindIndexBufferImpl(GpuBuffer* buffer, GpuSize offset, IndexType indexType)
    {
        ID3D11Buffer* d3dBuffer = static_cast<D3D11Buffer*>(buffer)->GetHandle();
        DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R16_UINT;
        if (indexType == IndexType::UInt32)
        {
            dxgiFormat = DXGI_FORMAT_R32_UINT;
        }

        _context->IASetIndexBuffer(d3dBuffer, dxgiFormat, static_cast<UINT>(offset));
    }

    void D3D11CommandBuffer::BindBufferImpl(GpuBuffer* buffer, uint32_t offset, uint32_t range, uint32_t set, uint32_t binding)
    {
        auto &b = _bindings.bindings[set][binding];
        if (b.buffer.buffer == buffer
            && b.buffer.offset == offset
            && b.buffer.range == range)
        {
            return;
        }

        b.buffer = { buffer, offset,  Align(range, 16u) };
        _dirtySets |= 1u << set;
    }

    void D3D11CommandBuffer::BindTextureImpl(Texture* texture, uint32_t set, uint32_t binding)
    {
        auto d3d11Texture = static_cast<D3D11Texture*>(texture);
        ID3D11ShaderResourceView* shaderResourceView = d3d11Texture->GetShaderResourceView();
        ID3D11SamplerState* samplerState = d3d11Texture->GetSamplerState();

        /*if (stage & ShaderStage::Compute)
        {
            _context->CSSetShaderResources(binding, 1, &shaderResourceView);
            _context->CSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Vertex)
        {
            _context->VSSetShaderResources(binding, 1, &shaderResourceView);
            _context->VSSetSamplers(binding, 1, &samplerState);
        }

        if (stage & ShaderStage::Fragment)*/
        {
            _context->PSSetShaderResources(binding, 1, &shaderResourceView);
            _context->PSSetSamplers(binding, 1, &samplerState);
        }
    }

    bool D3D11CommandBuffer::PrepareDraw(PrimitiveTopology topology)
    {
        uint32_t updateVboMask = _dirtyVbos/* & _currentPipeline->GetBindingMask()*/;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
            UpdateVbos(binding, count);
        });
        _dirtyVbos &= ~updateVboMask;

        if (_currentTopology != topology)
        {
            _context->IASetPrimitiveTopology(d3d::Convert(topology));
            _currentTopology = topology;
        }

        _currentShader->Bind(_context);

        /*auto rasterizerState = _currentPipeline->GetD3DRasterizerState();
        if (_currentRasterizerState != rasterizerState)
        {
            _currentRasterizerState = rasterizerState;
            _context->RSSetState(rasterizerState);
        }

        auto dsState = _currentPipeline->GetD3DDepthStencilState();
        if (_currentDepthStencilState != dsState)
        {
            _currentDepthStencilState = dsState;
            _context->OMSetDepthStencilState(dsState, 0);
        }

        auto blendState = _currentPipeline->GetD3DBlendState();
        if (_currentBlendState != blendState)
        {
            _currentBlendState = blendState;
            _context->OMSetBlendState(blendState, nullptr, 0xFFFFFFFF);
        }*/

        FlushDescriptorSets();

        return true;
    }

    void D3D11CommandBuffer::UpdateVbos(uint32_t binding, uint32_t count)
    {
#ifdef ALIMER_DEV
        for (uint32_t i = binding; i < binding + count; i++)
        {
            ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
        }
#endif
        _context->IASetVertexBuffers(
            binding,
            count,
            _vbo.d3dBuffers + binding,
            _vbo.strides + binding,
            _vbo.offsets + binding);

        // InputLayout
        if (_inputLayoutDirty)
        {
            _inputLayoutDirty = false;

            InputLayoutDesc newInputLayout;
            newInputLayout.first = reinterpret_cast<uint64_t>(_currentVertexInputFormat);
            newInputLayout.second = reinterpret_cast<uint64_t>(_currentShader->GetShader(ShaderStage::Vertex));

            if (_currentInputLayout != newInputLayout)
            {
                // Check if layout already exists
                auto inputLayout = _graphics->GetInputLayout(newInputLayout);
                if (inputLayout != nullptr)
                {
                    _context->IASetInputLayout(inputLayout);
                    _currentInputLayout = newInputLayout;
                }
                else
                {
                    // Not found, create new.
                    std::vector<D3D11_INPUT_ELEMENT_DESC> d3dElementDescs;
                    if (_currentVertexInputFormat)
                    {
                        d3dElementDescs = _currentVertexInputFormat->GetInputElements();
                    }
                    else
                    {
                        // Generate vertex input format from shader reflection.
                        std::vector<PipelineResource> inputs;

                        // Only consider input resources to vertex shader stage.
                        auto& resources = _currentShader->GetShader(ShaderStage::Vertex)->GetResources();
                        for (auto& resource : resources)
                        {
                            if (resource.resourceType == ResourceParamType::Input)
                            {
                                inputs.push_back(resource);
                            }
                        }

                        // Sort the vertex inputs by location value.
                        std::sort(inputs.begin(), inputs.end(), [](const PipelineResource& a, const PipelineResource& b) {
                            return (a.location < b.location);
                        });

                        // Create the VkVertexInputAttributeDescription objects.
                        uint32_t stride = 0;
                        for (auto& input : inputs)
                        {
                            // Assume all attributes are interleaved in the same buffer bindings.
                            D3D11_INPUT_ELEMENT_DESC inputElement = {};
                            inputElement.SemanticName = "TEXCOORD";
                            inputElement.SemanticIndex = input.location;
                            inputElement.Format;
                            inputElement.InputSlot = 0;
                            inputElement.AlignedByteOffset = stride;
                            inputElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                            inputElement.InstanceDataStepRate = 0;

                            uint32_t typeSize = 0;
                            switch (input.dataType)
                            {
                            case ParamDataType::Char:
                                typeSize = sizeof(char);
                                if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R8_SINT;
                                else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R8G8_SINT;
                                //else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R8G8B8_SINT;
                                else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R8G8B8A8_SINT;
                                break;

                            case ParamDataType::Int:
                                typeSize = sizeof(int32_t);
                                if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_SINT;
                                else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_SINT;
                                else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_SINT;
                                else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                                break;

                            case ParamDataType::UInt:
                                typeSize = sizeof(uint32_t);
                                if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_UINT;
                                else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_UINT;
                                else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_UINT;
                                else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                                break;

                            case ParamDataType::Half:
                                typeSize = sizeof(uint16_t);
                                if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R16_FLOAT;
                                else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R16G16_FLOAT;
                                //else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R16G16B16_FLOAT;
                                else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                                break;

                            case ParamDataType::Float:
                                typeSize = sizeof(float);
                                if (input.vecSize == 1) inputElement.Format = DXGI_FORMAT_R32_FLOAT;
                                else if (input.vecSize == 2) inputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
                                else if (input.vecSize == 3) inputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                                else if (input.vecSize == 4) inputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                                break;

                            default:
                                continue;
                            }

                            // Add the attribute description to the list.
                            d3dElementDescs.push_back(inputElement);

                            // Increase the stride for the next vertex attribute.
                            stride += typeSize * input.vecSize;
                        }
                    }

                    ID3DBlob* vsBlob = _currentShader->GetVertexShaderBlob();
                    ID3D11InputLayout* d3dInputLayout = nullptr;

                    if (FAILED(_graphics->GetD3DDevice()->CreateInputLayout(
                        d3dElementDescs.data(),
                        static_cast<UINT>(d3dElementDescs.size()),
                        vsBlob->GetBufferPointer(),
                        vsBlob->GetBufferSize(),
                        &d3dInputLayout)))
                    {
                        ALIMER_LOGERROR("D3D11 - Failed to create input layout");
                    }
                    else
                    {
                        _graphics->StoreInputLayout(newInputLayout, d3dInputLayout);
                        _context->IASetInputLayout(d3dInputLayout);
                        _currentInputLayout = newInputLayout;
                    }
                }
            }
        }
    }

    void D3D11CommandBuffer::FlushDescriptorSet(uint32_t set)
    {
        // TODO: We need to remap from vulkan to d3d11 binding scheme.
        if (_needWorkaround)
        {
            // Workaround for command list emulation
            ID3D11Buffer* NullCBuf = nullptr;
            _context->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, &NullCBuf);
        }

        ID3D11Buffer* buffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
        UINT offsets[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
        UINT ranges[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};

        for (uint32_t binding = 0; binding < MaxBindingsPerSet; ++binding)
        {
            auto &b = _bindings.bindings[set][binding];

            if (b.buffer.buffer == nullptr)
                continue;

            buffers[binding] = static_cast<D3D11Buffer*>(b.buffer.buffer)->GetHandle();
            offsets[binding] = b.buffer.offset;
            ranges[binding] = b.buffer.range;
        }

        _context->VSSetConstantBuffers1(
            0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT,
            buffers, offsets, ranges);
    }

    void D3D11CommandBuffer::FlushDescriptorSets()
    {
        uint32_t updateSet = 1; // layout.descriptorSetMask & _dirtySets;
        ForEachBit(updateSet, [&](uint32_t set) { FlushDescriptorSet(set); });
        _dirtySets &= ~updateSet;
    }

    void D3D11CommandBuffer::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->Draw(vertexCount, vertexStart);
        }
        else
        {
            _context->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
        }
    }

    void D3D11CommandBuffer::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!PrepareDraw(topology))
            return;

        if (instanceCount <= 1)
        {
            _context->DrawIndexed(indexCount, startIndex, 0);
        }
        else
        {
            _context->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0, 0);
        }
    }
}

