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

#include "CommandContextD3D12.h"
#include "GraphicsDeviceD3D12.h"
#include "D3D12CommandListManager.h"
#include "TextureD3D12.h"
#include "FramebufferD3D12.h"
#include "BufferD3D12.h"
#include "../D3D/D3DConvert.h"
#include "../../Core/Log.h"

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
	| D3D12_RESOURCE_STATE_COPY_DEST \
	| D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace alimer
{
    D3D12_COMMAND_LIST_TYPE GetD3D12CommandListType(QueueType type)
    {
        switch (type)
        {
        case QueueType::Graphics:
        case QueueType::AsyncGraphics:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;

        case QueueType::Compute:
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;

        case QueueType::Transfer:
            return D3D12_COMMAND_LIST_TYPE_COPY;

        default:
            ALIMER_UNREACHABLE();
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    D3D12CommandContext::D3D12CommandContext(GraphicsDeviceD3D12* device, QueueType type)
        : CommandContext(device, type)
        , _manager(device->GetCommandListManager())
        , _commandList(nullptr)
        , _currentAllocator(nullptr)
        , _type(GetD3D12CommandListType(type))
        , _graphicsState(device)
    {
        device->GetCommandListManager()->CreateNewCommandList(_type, &_commandList, &_currentAllocator);
    }

    D3D12CommandContext::~D3D12CommandContext()
    {
        SafeRelease(_commandList);
    }

    void D3D12CommandContext::Reset()
    {
        ALIMER_ASSERT(_commandList != nullptr && _currentAllocator == nullptr);
        _currentAllocator = _manager->GetQueue(_type).RequestAllocator();
        _commandList->Reset(_currentAllocator, nullptr);

        /* Reset cache states.*/
        _graphicsState.Reset();
        _numBarriersToFlush = _boundRTVCount = 0;
        _currentFramebuffer = nullptr;
        _currentD3DPipeline = nullptr;
    }

    void D3D12CommandContext::FlushImpl(bool waitForCompletion)
    {
        FlushResourceBarriers();

        //if (!_name.IsEmpty())
        //{
        //    //GpuProfiler::EndBlock(this);
        //}

        ALIMER_ASSERT(_currentAllocator != nullptr);

        // Execute the command list.
        auto& queue = _manager->GetQueue(_type);
        _fenceValue = queue.ExecuteCommandList(_commandList);
        const bool releaseContext = false;
        if (releaseContext)
        {
            queue.DiscardAllocator(_fenceValue, _currentAllocator);
            _currentAllocator = nullptr;
        }

        if (waitForCompletion)
        {
            _manager->WaitForFence(_fenceValue);
        }

        // Reset the command list and restore previous state
        if (!releaseContext)
        {
            _commandList->Reset(_currentAllocator, nullptr);
        }
        else
        {
            // Free context.
            //_device->FreeContext(this);
        }
    }

    void D3D12CommandContext::PushDebugGroupImpl(const std::string& name, const Color4& color)
    {
        /* TODO: Use pix3 */
    }

    void D3D12CommandContext::PopDebugGroupImpl()
    {
        /* TODO: Use pix3 */
    }

    void D3D12CommandContext::InsertDebugMarkerImpl(const std::string& name, const Color4& color)
    {
        /* TODO: Use pix3 */
    }

    void D3D12CommandContext::TransitionResource(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
    {
        D3D12_RESOURCE_STATES oldState = resource->GetUsageState();

        if (_type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        {
            ALIMER_ASSERT((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
            ALIMER_ASSERT((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
        }

        if (oldState != newState)
        {
            ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource->GetResource();
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldState;
            barrierDesc.Transition.StateAfter = newState;

            // Check to see if we already started the transition
            if (newState == resource->GetTransitioningState())
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                resource->SetTransitioningState((D3D12_RESOURCE_STATES)-1);
            }
            else
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            }

            resource->SetUsageState(newState);
        }
        else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            InsertUAVBarrier(resource, flushImmediate);
        }

        if (flushImmediate || _numBarriersToFlush == 16)
        {
            FlushResourceBarriers();
        }
    }

    void D3D12CommandContext::BeginResourceTransition(D3D12Resource* resource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
    {
        // If it's already transitioning, finish that transition
        if (resource->GetTransitioningState() != (D3D12_RESOURCE_STATES)-1)
            TransitionResource(resource, resource->GetTransitioningState());

        D3D12_RESOURCE_STATES oldState = resource->GetUsageState();

        if (oldState != newState)
        {
            ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
            D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource->GetResource();
            barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldState;
            barrierDesc.Transition.StateAfter = newState;
            barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;

            resource->SetTransitioningState(newState);
        }

        if (flushImmediate || _numBarriersToFlush == 16)
            FlushResourceBarriers();
    }

    void D3D12CommandContext::InsertUAVBarrier(D3D12Resource* resource, bool flushImmediate)
    {
        ALIMER_ASSERT(_numBarriersToFlush < 16 && "Exceeded arbitrary limit on buffered barriers");
        D3D12_RESOURCE_BARRIER& barrierDesc = _resourceBarrierBuffer[_numBarriersToFlush++];

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.UAV.pResource = resource->GetResource();

        if (flushImmediate)
            FlushResourceBarriers();
    }

    void D3D12CommandContext::FlushResourceBarriers()
    {
        if (_numBarriersToFlush > 0)
        {
            _commandList->ResourceBarrier(_numBarriersToFlush, _resourceBarrierBuffer);
            _numBarriersToFlush = 0;
        }
    }

#if TODO_D3D12
    void D3D12CommandContext::BeginRenderPassImpl(Framebuffer* framebuffer, const RenderPassBeginDescriptor* descriptor)
    {
        _currentFramebuffer = static_cast<D3D12Framebuffer*>(framebuffer->GetImpl());
        _boundRTVCount = framebuffer->GetColorAttachmentsCount();

        for (uint32_t i = 0; i < _boundRTVCount; ++i)
        {
            // Indicate that the resource will be used as a render target.
            _boundRTVResources[i] = static_cast<D3D12Texture*>(framebuffer->GetColorTexture(i)->GetImpl());
            TransitionResource(
                _boundRTVResources[i],
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                true);

            const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
            _commandList->ClearRenderTargetView(_currentFramebuffer->GetRTV(i), clearColor, 0, nullptr);
        }

        // Get first rtv handle.
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _currentFramebuffer->GetRTV(0);
        if (framebuffer->HasDepthStencilAttachment())
        {
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _currentFramebuffer->GetDSV();
            _commandList->OMSetRenderTargets(_boundRTVCount, &rtvHandle, FALSE, &dsvHandle);
        }
        else
        {
            _commandList->OMSetRenderTargets(_boundRTVCount, &rtvHandle, FALSE, nullptr);
        }

        // Set viewport and scissor to framebuffer size.
        /*uint32_t width = framebuffer->width;
        uint32_t height = framebuffer->height;
        AgpuViewport viewport = {
            0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
        AgpuRect2D scissorRect = { 0, 0, width, height };

        CmdSetViewport(viewport);
        CmdSetScissor(scissorRect);*/
    }

    void D3D12CommandContext::EndRenderPassImpl()
    {
        for (uint32_t i = 0; i < _boundRTVCount; ++i)
        {
            TransitionResource(_boundRTVResources[i], D3D12_RESOURCE_STATE_COMMON, true);
        }
    }

    void D3D12CommandContext::SetIndexBufferImpl(IndexBuffer* buffer, uint32_t offset, IndexType indexType)
    {
        D3D12Buffer* d3d12Buffer = static_cast<D3D12Buffer*>(buffer->GetImpl());

        D3D12_INDEX_BUFFER_VIEW bufferView;
        bufferView.BufferLocation = d3d12Buffer->GetGpuVirtualAddress() + offset;
        bufferView.SizeInBytes = static_cast<UINT>(buffer->GetSize() - offset);
        bufferView.Format = d3d::Convert(indexType);
        _commandList->IASetIndexBuffer(&bufferView);
    }

    void D3D12CommandContext::SetPrimitiveTopologyImpl(PrimitiveTopology topology)
    {
        _graphicsState.SetPrimitiveTopology(topology);
    }

    void D3D12CommandContext::DrawImpl(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        FlushGraphicsState();
        _commandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void D3D12CommandContext::FlushGraphicsState()
    {
        uint32_t active_vbos = 1;

        // We've invalidated pipeline state, update the VkPipeline.
        if (_graphicsState.IsDirty())
        {
            /*ID3D12PipelineState* oldPipelineState = _currentPipelineState;
            Hasher hasher;

            _currentPipelineState = _currentShader->d3d12PipelineState;
            if (oldPipelineState != _currentPipelineState)
            {
                if (_currentShader->isCompute)
                {
                    _commandList->SetComputeRootSignature(_currentShader->d3d12RootSignature);
                }
                else
                {
                    _commandList->SetGraphicsRootSignature(_currentShader->d3d12RootSignature);
                }

                _commandList->SetPipelineState(_currentPipelineState);
            }*/

            // Set primitive topology 
            D3D_PRIMITIVE_TOPOLOGY d3dPrimitiveTopology = d3d::Convert(_graphicsState.GetPrimitiveTopology(), 1);
            _commandList->IASetPrimitiveTopology(d3dPrimitiveTopology);

            // Reset graphics state dirty bit.
            _graphicsState.ClearDirty();
        }


        uint32_t update_vbo_mask = _dirtyVbos & active_vbos;
        /*ForEachBitRange(update_vbo_mask, [&](uint32_t binding, uint32_t count)
        {
#ifdef ALIMER_DEV
            for (uint32_t i = binding; i < binding + count; i++)
            {
                ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
            }
#endif

            _commandList->IASetVertexBuffers(binding, count, _d3dVbViews);
        });*/

        _dirtyVbos &= ~update_vbo_mask;
    }

    void D3D12CommandContext::SetPipeline(const SharedPtr<PipelineState>& pipeline)
    {
        _currentPipeline = StaticCast<D3D12PipelineState>(pipeline);
    }

    void D3D12CommandContext::DrawCore(PrimitiveTopology topology, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexStart, uint32_t baseInstance)
    {
        if (!PrepareDraw(topology))
            return;

        _commandList->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void D3D12CommandContext::DrawIndexedCore(PrimitiveTopology topology, uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex)
    {
        if (!PrepareDraw(topology))
            return;

        _commandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, 0, 0);
    }

    void D3D12CommandContext::FlushGraphicsPipelineState()
    {
        Hasher h;

        // TODO:
        uint32_t activeVbos = 1u << 0;
        //auto &layout = current_layout->get_resource_layout();
        //ForEachBit(layout.attribute_mask, [&](uint32_t bit) {
        //	h.u32(bit);
            //_activeVbos |= 1u << attribs[bit].binding;
        //	h.u32(attribs[bit].binding);
        //	h.u32(attribs[bit].format);
        //	h.u32(attribs[bit].offset);
        //});

        ForEachBit(activeVbos, [&](uint32_t bit) {
            h.u32(static_cast<uint32_t>(_vbo.inputRates[bit]));
            h.u32(_vbo.strides[bit]);
        });
    }

    bool D3D12CommandContext::PrepareDraw(PrimitiveTopology topology)
    {
        // We've invalidated pipeline state, update the VkPipeline.
        if (GetAndClear(
            /*COMMAND_BUFFER_DIRTY_STATIC_STATE_BIT
            | COMMAND_BUFFER_DIRTY_PIPELINE_BIT
            | */COMMAND_BUFFER_DIRTY_STATIC_VERTEX_BIT))
        {
            ID3D12PipelineState* oldPipelineState = _currentPipeline->GetD3DPipelineState();
            FlushGraphicsPipelineState();
            if (oldPipelineState != _currentD3DPipeline)
            {
                _currentPipeline->Bind(_commandList);
                //SetDirty(COMMAND_BUFFER_DYNAMIC_BITS);
            }
        }

        if (_currentTopology != topology)
        {
            _commandList->IASetPrimitiveTopology(d3d12::Convert(topology));
            _currentTopology = topology;
        }

        FlushDescriptorSets();

        const uint32_t activeVbos = 1u << 0;
        uint32_t updateVboMask = _dirtyVbos & activeVbos;
        ForEachBitRange(updateVboMask, [&](uint32_t binding, uint32_t count)
        {
            static D3D12_VERTEX_BUFFER_VIEW views[MaxVertexBufferBindings] = {};

            for (uint32_t i = binding; i < binding + count; i++)
            {
#ifdef ALIMER_DEV
                ALIMER_ASSERT(_vbo.buffers[i] != nullptr);
#endif

                views[i].BufferLocation = static_cast<D3D12GpuBuffer*>(_vbo.buffers[i])->GetGpuVirtualAddress();
                views[i].StrideInBytes = _vbo.strides[i];
                views[i].SizeInBytes = _vbo.buffers[i]->GetSize();
            }

            _commandList->IASetVertexBuffers(binding, count, views);
        });
        _dirtyVbos &= ~updateVboMask;

        return true;
    }

    void D3D12CommandContext::FlushDescriptorSets()
    {
        //auto &layout = current_layout->get_resource_layout();
        uint32_t setUpdate = 1; // layout.descriptor_set_mask & _dirtySets;
        ForEachBit(setUpdate, [&](uint32_t set) { FlushDescriptorSet(set); });
        _dirtySets &= ~setUpdate;
    }

    void D3D12CommandContext::FlushDescriptorSet(uint32_t set)
    {
        ResourceBinding binding = _bindings.bindings[set][0];

        D3D12_CONSTANT_BUFFER_VIEW_DESC bufferViewDesc;
        bufferViewDesc.BufferLocation = static_cast<const D3D12GpuBuffer*>(binding.buffer.buffer)->GetGpuVirtualAddress();
        bufferViewDesc.SizeInBytes = Align(static_cast<uint32_t>(binding.buffer.range), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        auto cpuHandle = _currentPipeline->GetCBVHeap()->GetCPUDescriptorHandleForHeapStart();
        _d3dDevice->CreateConstantBufferView(&bufferViewDesc, cpuHandle);

        _commandList->SetGraphicsRootDescriptorTable(
            set,
            _currentPipeline->GetCBVHeap()->GetGPUDescriptorHandleForHeapStart());
    }
#endif // TODO_D3D12

}
