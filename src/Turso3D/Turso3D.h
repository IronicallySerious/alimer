//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

// Base
#include "Base/HashSet.h"
#include "Base/List.h"
#include "Base/Ptr.h"

// Debug
#include "Debug/Log.h"
#include "Debug/Profiler.h"

// Thread
#include "Thread/Condition.h"
#include "Thread/Mutex.h"
#include "Thread/Thread.h"
#include "Thread/Timer.h"

// IO
#include "IO/Arguments.h"
#include "IO/Console.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/MemoryBuffer.h"
#include "IO/VectorBuffer.h"

// Object
#include "Object/Serializable.h"

// Math
#include "Math/Frustum.h"
#include "Math/Polyhedron.h"
#include "Math/Random.h"
#include "Math/Ray.h"

// Resource
#include "Resource/Image.h"
#include "Resource/JSONFile.h"
#include "Resource/ResourceCache.h"

// Grahics
#include "Graphics/ConstantBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderVariation.h"
#include "Graphics/Texture.h"
#include "Graphics/VertexBuffer.h"

// Scene
#include "Scene/SpatialNode.h"
#include "Scene/Scene.h"

// Renderer
#include "Renderer/Camera.h"
#include "Renderer/Light.h"
#include "Renderer/Material.h"
#include "Renderer/Model.h"
#include "Renderer/Octree.h"
#include "Renderer/Renderer.h"
#include "Renderer/StaticModel.h"


// Application
#include "Application/Input.h"
#include "Application/Window.h"
#include "Application/Application.h"
