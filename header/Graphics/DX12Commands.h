#pragma once
#include "pch.h"
#include "Graphics/Window.h"

class DX12Commands
{
public:
	DX12Commands(D3D12_COMMAND_LIST_TYPE type);
	~DX12Commands();

	void ExecuteCommandList(uint32_t frameIndex = 0);
	void ResetCommand(uint32_t frameIndex = 0);

	void WaitGpu(uint32_t timeout, uint32_t frameIndex);

	ComPtr<ID3D12CommandQueue> GetCommandQueue() { return m_pCommandQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCommandList() { return m_pCommandList;}

private:
	void CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type);
	void CreateCommandList();
	void CreateCommandAllocators();
	void CreateFence();

	ID3D12Device* m_pDevice = nullptr;

	/// <summary>
	/// コマンドアロケータ
	/// </summary>
	ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[Window::FrameCount];
	/// <summary>
	/// コマンドリスト
	/// </summary>
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
	/// <summary>
	/// コマンドキュー
	/// </summary>
	ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	/// <summary>
	/// フェンス
	/// </summary>
	ComPtr<ID3D12Fence> m_pFence;
	/// <summary>
	/// フェンスイベント
	/// </summary>
	HANDLE m_FenceEvent;
	/// <summary>
	/// フェンスカウンター
	/// </summary>
	uint64_t m_FenceCounter[Window::FrameCount];
};