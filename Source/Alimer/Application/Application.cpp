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

#include "AlimerVersion.h"
#include "../Application/Application.h"
#include "../Scene/Systems/CameraSystem.h"
#include "../IO/Path.h"
#include "../Core/Platform.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
    static Application* __appInstance = nullptr;

    Application::Application()
        : _running(false)
        , _paused(false)
        , _headless(false)
        , _settings{}
        , _log(new Logger())
        , _entities{}
        , _systems(_entities)
        , _scene(_entities)
    {
        PlatformConstruct();
        __appInstance = this;
    }

    Application::~Application()
    {
        _paused = true;
        _running = false;

        PluginManager::DeleteInstance();

        __appInstance = nullptr;
    }

    Application* Application::GetInstance()
    {
        return __appInstance;
    }

    bool Application::InitializeBeforeRun()
    {
        SetCurrentThreadName("Main");

        ALIMER_LOGINFOF("Initializing engine %s...", ALIMER_VERSION_STR);

        // Init Window and Gpu.
        if (!_headless)
        {
            _window = MakeWindow("Alimer",
                _settings.renderingSettings.defaultBackBufferWidth,
                _settings.renderingSettings.defaultBackBufferHeight);

            // Assign as window handle.
            _settings.renderingSettings.windowHandle = _window->GetHandle();

            // Create and init graphics.
            _graphicsDevice = GraphicsDevice::Create(_settings.preferredGraphicsBackend, _settings.validation);
            if (!_graphicsDevice->Initialize(_settings.renderingSettings))
            {
                ALIMER_LOGERROR("Failed to initialize GraphicsDevice.");
                return false;
            }
        }

        // Create per platform Input module.
        _input = CreateInput();

        // Create per platform Audio module.
        _audio = CreateAudio();

        // Load plugins
        LoadPlugins();

        // Initialize this instance and all systems.
        Initialize();

        // Setup and configure all systems.
        _systems.Add<CameraSystem>();
        _renderContext.SetDevice(_graphicsDevice.Get());

        ALIMER_LOGINFO("Engine initialized with success.");
        _running = true;
        //BeginRun();

        // Reset timer.
        _timer.Reset();

        // Run the first time an update
        //InternalUpdate();

        return true;
    }

    void Application::LoadPlugins()
    {
        PluginManager::GetInstance()->LoadPlugins(GetExecutableFolder());
    }

    void Application::RunFrame()
    {
        if (!_paused)
        {
            // Tick timer.
            double frameTime = _timer.Frame();
            double deltaTime = _timer.GetElapsed();

            // Update all systems.
            _systems.Update(deltaTime);

            // Render single frame.
            if (!_window->IsMinimized())
            {
                RenderFrame(frameTime, deltaTime);
            }
        }

        // Update input, even when paused.
        _input->Update();
    }

    void Application::RenderFrame(double frameTime, double elapsedTime)
    {
        if (_headless)
            return;

        if (!_graphicsDevice->BeginFrame())
            return;

        CommandBuffer* commandBuffer = _graphicsDevice->GetMainCommandBuffer();

        RenderPassDescriptor renderPass = {};
        renderPass.colorAttachments[0].clearColor = Color4(0.0f, 0.2f, 0.4f, 1.0f);
        commandBuffer->BeginRenderPass(&renderPass);
        commandBuffer->EndRenderPass();

        /*

        // Begin recording.
        commandBuffer->Begin();
        commandBuffer->BeginRenderPass(nullptr, Color4(0.0f, 0.2f, 0.4f, 1.0f));

        // Call OnRenderFrame for custom rendering frame logic.
        OnRenderFrame(commandBuffer, frameTime, elapsedTime);

        // Render scene to default command buffer.
        if (_renderPipeline)
        {
            //auto camera = _scene.GetActiveCamera()->GetComponent<CameraComponent>()->camera;
            //_renderPipeline->Render(_renderContext, { camera });
        }

        // End swap chain render pass.
        commandBuffer->EndRenderPass();

        // End recording.
        commandBuffer->End();
        */
        // Commit rendering frame.
        _graphicsDevice->EndFrame();
    }

    void Application::OnRenderFrame(CommandBuffer* commandBuffer, double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        // By default clear with some color.
        //commandBuffer->BeginRenderPass(nullptr, Color4(0.0f, 0.2f, 0.4f, 1.0f));
        //commandBuffer->EndRenderPass();
    }

    void Application::Exit()
    {
        _paused = true;

        if (_running)
        {
            // TODO: Fire event.
            _running = false;
        }
    }

    void Application::Pause()
    {
        if (_running && !_paused)
        {
            // TODO: Fire event.
            _paused = true;
        }
    }

    void Application::Resume()
    {
        if (_running && _paused)
        {
            // TODO: Fire event.
            _paused = false;
        }
    }

    Application& gApplication()
    {
        return *__appInstance;
    }
}
