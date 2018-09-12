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

#include "../Scene/ComponentSystem.h"
#include "../Scene/Scene.h"

namespace Alimer
{
    uint32_t Detail::IDMapping::_systemIds;

    void SystemManager::AddToSystems(Entity entity)
    {
       /* const auto& entMask = entity.getComponentMask();
        for (auto& pair : _systems)
        {
            const auto& sysMask = sys->getComponentMask();
            if ((entMask & sysMask) == sysMask)
            {
                pair.second->AddEntity(entity);
            }
        }*/
    }

    void SystemManager::Configure()
    {
        for (auto &pair : _systems)
        {
            pair.second->Configure(_entityManager/*, _eventManager*/);
        }
        _initialized = true;
    }

    void SystemManager::Update(double deltaTime)
    {
        assert(_initialized && "SystemManager::Configure() must be called first");
        for (auto &pair : _systems)
        {
            pair.second->Update(_entityManager, /*_eventManager,*/ deltaTime);
        }
    }
}
