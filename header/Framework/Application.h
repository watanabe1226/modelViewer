#pragma once
#include "pch.h"
#include "Math/Matrix4x4.h"
class Application
{
public:
	Application(uint32_t width, uint32_t height);
	~Application();
	void Run();

	struct alignas(256) Transform
	{
		Matrix4x4 World; // ワールド変換行列
		Matrix4x4 View;  // ビュー変換行列
		Matrix4x4 Proj;  // プロジェクション変換行列
	};

	template<typename T>
	struct ConstantBufferView
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC Desc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU = {};
		D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU = {};
		T* pBuffer = nullptr;
	};
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
	ComPtr<ID3D12Device> m_pDevice;
	/// <summary>
	/// コマンドキュー
	/// </summary>
	ComPtr<ID3D12CommandQueue> m_pQueue;
	/// <summary>
	/// スワップチェーン
	/// </summary>
	ComPtr<IDXGISwapChain3> m_pSwapChain;
	/// <summary>
	/// カラーバッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];
	/// <summary>
	/// カラーバッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pDepthBuffer;
	/// <summary>
	/// コマンドアロケータ
	/// </summary>
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];
	/// <summary>
	/// コマンドリスト
	/// </summary>
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;
	/// <summary>
	/// ディスクリプタヒープ(RTV)
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;
	/// <summary>
	/// ディスクリプタヒープ(DSV)
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;
	/// <summary>
	/// ディスクリプタヒープ(CBV)
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> m_pHeapCBV;
	/// <summary>
	/// 頂点バッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pVB;
	/// <summary>
	/// インデックスバッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pIB;
	/// <summary>
	/// 定数バッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pCB[FrameCount];
	/// <summary>
	/// ルートシグネチャ
	/// </summary>
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	/// <summary>
	/// パイプラインステート
	/// </summary>
	ComPtr<ID3D12PipelineState> m_pPSO;
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
	///  CPUディスクリプターハンドル(DSV)
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE m_HandleDSV;
	/// <summary>
	/// 頂点バッファビュー
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW m_VBV;
	/// <summary>
	/// インデックスバッファビュー
	/// </summary>
	D3D12_INDEX_BUFFER_VIEW m_IBV;
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
	bool OnInit();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};