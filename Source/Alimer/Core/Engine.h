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

#include "Core/Window.h"
#include <new>
#include <memory>
#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <string>
#include <atomic>

void AlimerMain(const std::vector<std::string>& args);

namespace Alimer
{
	/// Application subsystem for main loop and all modules and OS setup.
	class Engine
	{
	protected:
		/// Construct and register subsystem.
		Engine();

	public:
		/// Destructor.
		virtual ~Engine();

		/// Runs main loop.
		virtual int Run();

		void Pause();
		void Resume();

		virtual std::shared_ptr<Window> CreateWindow() = 0;

	protected:
		virtual bool Initialize();
		virtual void RunMain();
		static bool SetCurrentThreadName(const std::string& name);

		std::vector<std::string> _args;
		std::atomic<bool> _running;
		std::atomic<bool> _paused;
		std::atomic<bool> _headless;

		std::shared_ptr<Window> _window;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Engine);
	};

	extern Engine* engine;
}
