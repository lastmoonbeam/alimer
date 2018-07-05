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

#include "Alimer.h"
using namespace Alimer;



namespace Alimer
{
    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override = default;

    private:
        void Initialize() override;
        void OnRender(RenderPass* frameRenderPass) override;
    };

    RuntimeApplication::RuntimeApplication()
    {
    }

    void RuntimeApplication::Initialize()
    {
        // Create scene
        auto triangleEntity = _scene->CreateEntity();
        triangleEntity->AddComponent<TransformComponent>();
        triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }

    void RuntimeApplication::OnRender(RenderPass* frameRenderPass)
    {
        CommandBuffer* commandBuffer = _graphics->GetDefaultCommandBuffer();
        Color clearColor = Color::Green;
        RenderPassCommandEncoder* encoder = commandBuffer->CreateRenderPassCommandEncoder(frameRenderPass, clearColor);
        encoder->EndEncoding();
        commandBuffer->Commit();
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
