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

#include "../Graphics/Sampler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GPUDevice.h"
#include "../Core/Log.h"

namespace Alimer
{
    Sampler::Sampler()
        : GraphicsResource(Graphics::GetInstancePtr())
    {

    }

    bool Sampler::Define(const SamplerDescriptor* descriptor)
    {
        ALIMER_ASSERT_MSG(descriptor, "Invalid descriptor");

        Destroy();

        _sampler = _graphics->GetGPUDevice()->CreateSampler(descriptor);
        if (_sampler == nullptr)
        {
            ALIMER_LOGERROR("Failed to define sampler.");
            return false;
        }

        memcpy(&_descriptor, descriptor, sizeof(SamplerDescriptor));
        ALIMER_LOGDEBUG("Sampler defined with success.");
        return true;
    }

    void Sampler::RegisterObject()
    {
        RegisterFactory<Sampler>();
    }
}
