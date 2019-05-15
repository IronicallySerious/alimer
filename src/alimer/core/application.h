//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "core/window.h"
//#include "input.hpp"
#include "content/content_manager.h"
#include <string>
#include <vector>
#include <memory>

namespace alimer
{
    class Graphics;

    /// Application for main setup and main loop.
    class ALIMER_API Application
    {
    public:
        Application();

        /// Destructor.
        virtual ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        inline const std::vector<std::string>& getArgs() const { return _args; }

        /// Get the main system.
        inline Window& get_window() { return _window; }

        /// Get the input system.
        //inline Input& get_input() { return _input; }

        /// Get the content manager.
        inline ContentManager& get_content() { return _content; }

    protected:
        // Initialize after all system setup
        void initialize();
        /// Run one frame.
        void frame();

    protected:
        std::vector<std::string> _args;

        /// Main window
        Window _window;

        uint32_t _width = 0;
        uint32_t _height = 0;

        /// Input system.
        //Input _input;

        /// Content manager
        ContentManager _content;

        /// Graphics system.
        std::unique_ptr<Graphics> _graphics;
    };
} 
