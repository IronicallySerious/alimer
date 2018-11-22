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

#include "../Resource/Resource.h"
#include "../Core/Log.h"

namespace Alimer
{
	Resource::Resource()
		: _asyncLoadState(AsyncLoadState::Done)
	{
	}

    bool Resource::Load(Stream& source)
    {
        //SetAsyncLoadState(Thread::IsMainThread() ? AsyncLoadState::Done : AsyncLoadState::Loading);
        SetAsyncLoadState(AsyncLoadState::Done);
        bool success = BeginLoad(source);
        if (success)
            success &= EndLoad();
        SetAsyncLoadState(AsyncLoadState::Done);

        return success;
    }

    bool Resource::BeginLoad(Stream&)
    {
        // This always needs to be overridden by subclasses
        return false;
    }

    bool Resource::EndLoad()
    {
        // Resources that do not need access to main-thread critical objects do not need to override this
        return true;
    }

    bool Resource::Save(Stream& dest)
    {
        ALIMER_LOGERRORF("Save not supported for '%s'", GetTypeName().CString());
        return false;
    }


	void Resource::SetName(const String& name)
	{
		_name = name;
		_nameHash = name;
	}

    void Resource::SetAsyncLoadState(AsyncLoadState newState)
    {
        _asyncLoadState = newState;
    }
}
