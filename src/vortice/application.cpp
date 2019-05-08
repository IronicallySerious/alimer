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
#include "gpu/vgpu.h"

namespace vortice
{
    Application::Application()
    {
        platform_construct();
    }

    Application::~Application()
    {
        // Shutdown vgpu
        vgpuShutdown();

        platform_shutdown();
    }

    int Application::run()
    {
        int result = run_main_loop();
        return result;
    }

    void Application::setup()
    {
        _window.define("vortice", _width, _height);

        // setup vgpu using window handle
        VGpuRendererSettings gpuDescriptor = {};
#if defined(_DEBUG)
        gpuDescriptor.validation = false;
#endif

#if defined(__linux__)
        gpuDescriptor.handle.connection = XGetXCBConnection(glfwGetX11Display());
        gpuDescriptor.handle.window = glfwGetX11Window(window);
#elif defined(_WIN32)
        gpuDescriptor.handle.hinstance = ::GetModuleHandleW(NULL);
        gpuDescriptor.handle.hwnd = _window.handle();
#endif
        gpuDescriptor.width = _width;
        gpuDescriptor.height = _height;
        gpuDescriptor.swapchain.imageCount = 3;
        gpuDescriptor.swapchain.srgb = true;
        gpuDescriptor.swapchain.colorClearValue = { 0.0f, 0.0f, 0.2f, 1.0f };
        gpuDescriptor.swapchain.depthStencilFormat = VGPU_PIXEL_FORMAT_D32_FLOAT;
        gpuDescriptor.swapchain.sampleCount = VGPU_SAMPLE_COUNT1;
        vgpuInitialize("vortice", &gpuDescriptor);

        const float vertices[] = {
            /* positions            colors */
             0.0f, 0.5f, 0.5f,      1.0f, 0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
        };

        VGpuBufferDescriptor buffer_desc = {};
        buffer_desc.usage = VGPU_BUFFER_USAGE_VERTEX;
        buffer_desc.size = sizeof(vertices);
        auto buffer = vgpuCreateBuffer(&buffer_desc, vertices);
    }

    void Application::frame()
    {
        if (vgpuBeginFrame() == VGPU_ERROR_BEGIN_FRAME_FAILED) {
            return;
        }

        /* Get frame command buffer for recording. */
        VGpuCommandBuffer commandBuffer = vgpuGetCommandBuffer();
        vgpuBeginCommandBuffer(commandBuffer);
        vgpuCmdBeginDefaultRenderPass(commandBuffer, { 0.5f, 0.5f, 0.5f, 1.0f}, 1.0f, 0);
        vgpuCmdEndRenderPass(commandBuffer);
        vgpuEndCommandBuffer(commandBuffer);
        vgpuSubmitCommandBuffer(commandBuffer);

        vgpuEndFrame();

        /*sg_pass_action pass_action = {};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        //pass_action.colors[0].val = { 0.0f, 0.0f, 0.2f, 1.0f };

        sg_begin_default_pass(&pass_action, _width, _height);
        /*sg_apply_pipeline(pip);
        sg_apply_bindings(&bind);
        sg_draw(0, 3, 1);
        sg_end_pass();
        sg_commit();*/
    }

} // namespace vortice
