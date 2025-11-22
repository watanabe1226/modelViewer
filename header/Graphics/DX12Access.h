#pragma once
#include "pch.h"

class DX12Commands;
class DX12Device;
class DX12DescriptorHeap;
class Window;

namespace DX12Access
{
	DX12Commands* GetCommands(D3D12_COMMAND_LIST_TYPE type);
	ComPtr<ID3D12Device> GetDevice();
	DX12DescriptorHeap* GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
	Window* GetWindow();
}