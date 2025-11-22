#include "Graphics/DX12Commands.h"
#include "Graphics/DX12Access.h"
#include "Graphics/DX12Utilities.h"

DX12Commands::DX12Commands(D3D12_COMMAND_LIST_TYPE type)
{
	m_pDevice = DX12Access::GetDevice().Get();

	CreateCommandQueue(type);
	CreateCommandAllocators();
	CreateCommandList();
	CreateFence();
}

DX12Commands::~DX12Commands()
{
	// ハンドルを閉じる.
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	// フェンスオブジェクトを破棄.
	m_pFence.Reset();
}

void DX12Commands::ExecuteCommandList(uint32_t frameIndex)
{
	ThrowFailed(m_pCommandList->Close());

	// コマンドの実行
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pCommandQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);
	WaitGpu(INFINITY, frameIndex);
}

void DX12Commands::ResetCommand(uint32_t frameIndex)
{
	m_pCommandAllocators[frameIndex]->Reset();
	m_pCommandList->Reset(m_pCommandAllocators[frameIndex].Get(), nullptr);
}

void DX12Commands::WaitGpu(uint32_t timeout, uint32_t frameIndex)
{
	assert(m_pCommandQueue.Get() != nullptr);
	assert(m_pFence.Get() != nullptr);
	assert(m_FenceEvent != nullptr);

	const auto fenceValue = m_FenceCounter[frameIndex]; //! 現在のフレームのフェンスカウンターを取得

	// シグナル処理
	auto hr = m_pCommandQueue->Signal(m_pFence.Get(), fenceValue);
	ThrowFailed(hr);

	// カウンターを増やす
	++m_FenceCounter[frameIndex];

	// 次のフレーム描画処理がまだであれば待機
	if (m_pFence->GetCompletedValue() < fenceValue)
	{
		// 完了時にイベントを設定
		auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_FenceEvent);
		if (FAILED(hr))
		{
			return;
		}

		// 待機処理
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_FenceEvent, timeout, FALSE))
		{
			return;
		}
	}
}

/// <summary>
/// コマンドキューを作成します。
/// </summary>
void DX12Commands::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// GPUが1つを前提にしているため0
	desc.NodeMask = 0;

	auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pCommandQueue.GetAddressOf()));
	ThrowFailed(hr);
}

/// <summary>
/// コマンドリストを作成します。
/// </summary>
void DX12Commands::CreateCommandList()
{
	auto hr = m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCommandAllocators[0].Get(),
		nullptr,
		IID_PPV_ARGS(m_pCommandList.GetAddressOf())
	);
	ThrowFailed(hr);
}

/// <summary>
/// コマンドアロケータを作成します。
/// </summary>
void DX12Commands::CreateCommandAllocators()
{
	for (auto i = 0u; i < Window::FrameCount; ++i)
	{
		auto hr = m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_pCommandAllocators[i].GetAddressOf())
		);
		ThrowFailed(hr);
	}
}

/// <summary>
/// フェンスを作成します。
/// </summary>
void DX12Commands::CreateFence()
{
	for (uint32_t i = 0; i < Window::FrameCount; ++i)
	{
		m_FenceCounter[i] = 0;
	}

	// フェンスの生成
	auto hr = m_pDevice->CreateFence(
		m_FenceCounter[0],
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(m_pFence.GetAddressOf())
	);
	ThrowFailed(hr);

	// カウンター設定
	m_FenceCounter[0] = 1;

	// イベントの生成
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(m_FenceEvent != nullptr && "Failed to create Fence Event");
}
