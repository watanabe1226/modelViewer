#pragma once
#include "pch.h"
#include "Graphics/DX12Access.h"
#include "Graphics/Window.h"
#include "Graphics/DX12Device.h"
#include "Graphics/DX12Commands.h"

#define FILE_PREFIX __FILE__ "(" TO_STRING(__LINE__) "): " 
#define DX12MSGThrowIfFailed(hr, msg) DX12Utility::CheckResultCodeD3D12(hr, FILE_PREFIX msg)
#define DX12MSGThrowFailed(msg) DX12Utility::CheckCodeD3D12(msg FILE_PREFIX)
#define ThrowFailed(hr) DX12Utility::ThrowIfFailed(hr)

namespace DX12Utility
{
    class DX12Exception : public std::runtime_error
    {
    public:
        DX12Exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
    };

    inline void CheckResultCodeD3D12(HRESULT hr, const std::string errorMsg)
    {
        if (FAILED(hr))
            throw DX12Exception(errorMsg);
    }

    inline void CheckCodeD3D12(const std::string errorMsg)
    {
        throw DX12Exception(errorMsg);
    }

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }

    inline void TransitionResource(ID3D12Resource* resource, 
        D3D12_RESOURCE_STATES beforeState, 
        D3D12_RESOURCE_STATES afterState)
    {
		ID3D12GraphicsCommandList* pCmdList = DX12Access::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetGraphicsCommandList().Get();
        // リソースバリアの設定
        D3D12_RESOURCE_BARRIER barrior = {};
        barrior.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrior.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrior.Transition.pResource = resource;
        barrior.Transition.StateBefore = beforeState;
        barrior.Transition.StateAfter = afterState;
        barrior.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        // リソースバリア
        pCmdList->ResourceBarrier(1, &barrior);
    }

    inline void BindAndClearRenderTarget(Window* window, 
        D3D12_CPU_DESCRIPTOR_HANDLE* renderTarget, 
        D3D12_CPU_DESCRIPTOR_HANDLE* depthStencil = nullptr,
        float* clearColor = nullptr)
    {
        ID3D12GraphicsCommandList* pCmdList = DX12Access::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetGraphicsCommandList().Get();
        const float defaultClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        if (clearColor)
        {
            pCmdList->ClearRenderTargetView(*renderTarget, clearColor, 0, nullptr);
        }
        else
        {
            pCmdList->ClearRenderTargetView(*renderTarget, defaultClearColor, 0, nullptr);
        }


        if (depthStencil != nullptr)
        {
            pCmdList->ClearDepthStencilView(*depthStencil, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }

		auto viewport = window->GetViewport();
		auto scissor = window->GetScissorRect();
        pCmdList->RSSetViewports(1, &viewport);
        pCmdList->RSSetScissorRects(1, &scissor);
        pCmdList->OMSetRenderTargets(1, renderTarget, FALSE, depthStencil);
    }
}