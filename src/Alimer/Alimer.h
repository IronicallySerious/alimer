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

#include "foundation/Swap.h"
#include "foundation/Ptr.h"
#include "Base/Ptr.h"
#include "Base/String.h"
#include "Base/StringHash.h"

// Debug
#include "Debug/Debug.h"
#include "Core/Log.h"

// Core
#include "Core/Platform.h"
#include "Core/Plugin.h"

// IO
#include "IO/Stream.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"

// Math
#include "Math/MathUtil.h"
#include "Math/Math.h"
#include "Math/Vector2.h"
#include "Math/Color.h"
#include "Math/Rectangle.h"
#include "Math/Matrix4x4.h"

// Graphics
#include "Graphics/PixelFormat.h"
#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/GraphicsDevice.h"

// Resource
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"

// Serialization
#include "Serialization/Serializable.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonDeserializer.h"

// Scene
#include "Scene/Entity.h"
#include "Scene/Components/TransformComponent.h"
#include "Scene/Components/CameraComponent.h"
//#include "Scene/Components/Renderable.h"

// Renderer
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"

// Application
#include "Application/Application.h"
#include "Application/Window.h"

