#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Application
{
public:
	Application(uint32_t width, uint32_t height);
	~Application();
	void Run();

private:

	/// <summary>
	/// フレームバッファ数
	/// </summary>
	static const uint32_t FrameCount = 2;

	/// <summary>
	/// インスタンスハンドル
	/// </summary>
	HINSTANCE m_HInstance;
	/// <summary>
	/// ウィンドウハンドル
	/// </summary>
	HWND m_hWnd;
	/// <summary>
	/// ウィンドウの横幅
	/// </summary>
	uint32_t m_Width;
	/// <summary>
	/// ウィンドウの縦幅
	/// </summary>
	uint32_t m_Height;
	/// <summary>
	/// デバイス
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Device> m_pDevice;
	/// <summary>
	/// コマンドキュー
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pQueue;
	/// <summary>
	/// スワップチェーン
	/// </summary>
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_pSwapChain;
	/// <summary>
	/// カラーバッファ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];
	/// <summary>
	/// コマンドアロケータ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];
	/// <summary>
	/// コマンドリスト
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCmdList;
	/// <summary>
	/// ディスクリプタヒープ(RTV)
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	/// <summary>
	/// ディスクリプタヒープ(CBV)
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;
	/// <summary>
	/// 頂点バッファ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pVB;
	/// <summary>
	/// 定数バッファ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pCB[FrameCount];
	/// <summary>
	/// ルートシグネチャ
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;
	/// <summary>
	/// パイプラインステート
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSO;
	/// <summary>
	/// フェンス
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
	/// <summary>
	/// フェンスイベント
	/// </summary>
	HANDLE m_FenceEvent;
	/// <summary>
	/// フェンスカウンター
	/// </summary>
	uint64_t m_FenceCounter[FrameCount];
	/// <summary>
	/// フレーム番号
	/// </summary>
	uint32_t m_FrameIndex = 0;
	/// <summary>
	///  CPUディスクリプターハンドル(RTV)
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleRTV[FrameCount];
	/// <summary>
	/// 頂点バッファビュー
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW m_VBV;
	/// <summary>
	/// ビューポート
	/// </summary>
	D3D12_VIEWPORT m_Viewport;
	/// <summary>
	/// シザー矩形
	/// </summary>
	D3D12_RECT m_Scissor;
	/// <summary>
	/// 定数バッファビュー
	/// </summary>
	ConstantBufferView<Transform> m_CBV[FrameCount];
	/// <summary>
	/// 回転角
	/// </summary>
	float m_RotateAngle;
	
	bool Initialize();
	void TermApplication();
	bool InitWindow();
	void TermWindow();
	void MainLoop();
	bool InitD3D();
	void TermD3D();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};