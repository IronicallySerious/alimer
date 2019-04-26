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

#include "core/log.h"
#include "application.hpp"
#include "gpu/gpu.h"

namespace vortice
{
    Application::Application()
    {
        platform_construct();
    }

    Application::~Application()
    {
        // Shutdown vgpu
        vgpu_shutdown();

        platform_shutdown();
    }

    int Application::run()
    {
        int result = run_main_loop();
        return result;
    }

    void Application::setup()
    {
    }

    void Application::frame()
    {
        /*sg_pass_action pass_action = {};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        pass_action.colors[0].val = { { 0.0f, 0.0f, 0.2f, 1.0f } };

        sg_begin_default_pass(&pass_action, _width, _height);
        sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
        sg_draw(0, 3, 1);
        sg_end_pass();
        sg_commit();*/
    }

} // namespace vortice
