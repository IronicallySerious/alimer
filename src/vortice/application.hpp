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
#include "input.hpp"

namespace vortice
{
    class VORTICE_API Application
    {
    public:
        Application();

        /// Destructor.
        virtual ~Application();

        /// Runs main loop using Platform instance.
        int run();

        /// Get the input system.
        inline Input& get_input() { return _input; }

    private:
        void platform_construct();
        void platform_shutdown();
        /// Run OS main loop.
        int run_main_loop();

        void setup();
        /// Run one frame.
        void frame();

    protected:
        uint32_t _width = 1280;
        uint32_t _height = 720;

        /// Input system.
        Input _input;
    };
} // namespace vortice
