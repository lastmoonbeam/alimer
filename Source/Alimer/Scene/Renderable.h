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

#include "../Math/MathUtil.h"
#include "../Util/Intrusive.h"
#include "../Scene/Entity.h"
#include "../Graphics/Graphics.h"

namespace Alimer
{
    class ALIMER_API Renderable : public IntrusivePtrEnabled<Renderable>
    {
    public:
        virtual ~Renderable() = default;

        virtual void Render(CommandBuffer* commandBuffer) = 0;
    };

    using RenderableHandle = IntrusivePtr<Renderable>;

    class ALIMER_API RenderableComponent : public Component
    {
    public:
        RenderableHandle renderable;
    };

    class ALIMER_API TriangleRenderable final : public Renderable
    {
    public:
        TriangleRenderable();
        virtual ~TriangleRenderable();

        void Render(CommandBuffer* commandBuffer) override;
    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<PipelineState> _renderPipeline;
    };
}
