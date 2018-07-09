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

#include "../Scene/SceneManager.h"
#include "../Scene/Scene.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
    SceneManager::SceneManager()
    {
    }

    SceneManager::~SceneManager()
    {
    }

    void SceneManager::SetScene(Scene* scene)
    {
        assert(scene);

        if (scene->_sceneManager) {
            scene->_sceneManager->RemoveScene(scene);
        }

        scene->_sceneManager = this;

        _scenes.push_back(scene);
    }

    bool SceneManager::RemoveScene(Scene* scene)
    {
        bool result = false;

        // Lookup for scene.
        auto it = std::find(_scenes.begin(), _scenes.end(), scene);

        if (it != end(_scenes))
        {
            // TODO:
            // if (scene->_isEntered)
            //     scene->Leave();
            scene->_sceneManager = nullptr;
            _scenes.erase(it);

            result = true;
        }

        return result;
    }
}
