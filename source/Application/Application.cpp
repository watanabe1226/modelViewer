#include "Application.h"

namespace
{
	/// <summary>
	/// ウィンドウクラス名
	/// </summary>
	const auto ClassName = TEXT("ModelViewerWindow");
}

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="width"> ウィンドウの横幅 </param>
/// <param name="height"> ウィンドウの縦幅 </param>
Application::Application(uint32_t width, uint32_t height)
	: m_HInstance(nullptr),
	m_hWnd(nullptr),
	m_Width(width),
	m_Height(height)
{

}

/// <summary>
/// デストラクタ
/// </summary>
Application::~Application()
{
}

/// <summary>
/// 実行します．
/// </summary>
void Application::Run()
{
	if (Initialize())
	{
		MainLoop();
	}

	TermD3D();
	TermApplication();
}

/// <summary>
/// 初期化処理です．
/// </summary>
bool Application::Initialize()
{
	// ウィンドウの初期化
	if (!InitWindow())
	{
		return false;
	}

	// Direct3Dの初期化
	if (!InitD3D())
	{
		return false;
	}
	return true;
}

/// <summary>
/// アプリ終了処理を行います．
/// </summary>
void Application::TermApplication()
{
	// ウィンドウの終了処理
	TermWindow();
}

/// <summary>
/// ウィンドウの初期化処理を行います．
/// </summary>
bool Application::InitWindow()
{
	// インスタンスハンドルを取得
	auto hInst = GetModuleHandle(nullptr);
	if (hInst == nullptr)
	{
		return false;
	}

	// ウィンドウの設定
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX); // WNDCLASSEX構造体のサイズを設定
	wc.style = CS_HREDRAW | CS_VREDRAW; // ウィンドウのスタイルを設定
	wc.lpfnWndProc = WindowProc; // ウィンドウプロシージャの関数ポインタを設定
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // アプリケーションのアイコンを設定
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND); // 背景色を設定
	wc.lpszMenuName = nullptr; // メニュー名を設定しない
	wc.lpszClassName = ClassName; // ウィンドウクラス名を設定
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION); // 小さいアイコンを設定

	// ウィンドウクラスの登録
	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	// インスタンスハンドルの設定
	m_HInstance = hInst;

	// ウィンドウのサイズを設定
	RECT rect = {};
	rect.right = static_cast<LONG>(m_Width);
	rect.bottom = static_cast<LONG>(m_Height);

	// ウィンドウサイズを調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rect, style, FALSE);

	// ウィンドウを生成
	m_hWnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("Model Viewer"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		m_HInstance,
		nullptr
	);

	if (m_hWnd == nullptr)
	{
		return false;
	}

	// ウィンドウを表示
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// ウィンドウを更新
	UpdateWindow(m_hWnd);

	// ウィンドウにフォーカスを設定
	SetFocus(m_hWnd);

	return true;
}

/// <summary>
/// ウィンドウの終了処理を行います.
/// </summary>
void Application::TermWindow()
{
	// ウィンドウの登録を解除
	if (m_HInstance != nullptr)
	{
		UnregisterClass(ClassName, m_HInstance);
	}

	m_HInstance = nullptr;
	m_hWnd = nullptr;
}

/// <summary>
/// メインループです.
/// </summary>
void Application::MainLoop()
{
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE)
		{
			// メッセージを変換
			TranslateMessage(&msg);
			// メッセージをディスパッチ
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
}

/// <summary>
/// Direct3Dの初期化処理です．
/// </summary>
bool Application::InitD3D()
{
#if defined(DEBUG) || defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> debug;
	auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));
	
	// デバッグレイヤーを有効化
	if (SUCCEEDED(hr))
	{
		debug->EnableDebugLayer();
	}
#endif

	// デバイスの生成
	{
		auto hr = D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(m_pDevice.GetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}
	}

	// コマンドキューの生成
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		// GPUが実行可能なコマンドバッファを指定
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		// GPUが1つを前提にしているため0
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}

	// スワップチェーン
	{
		// DXGIファクトリーの生成
		Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// スワップチェーンの設定
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_Width;
		desc.BufferDesc.Height = m_Height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hWnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// スワップチェーンの生成
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// IDXGISwapChain3を取得
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		// バックバッファ番号を取得
		m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	}

	// コマンドアロケータの生成
	// コマンドリストが使用するメモリを割り当てるためのもの
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(m_pCmdAllocator[i].GetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}
		}
	}

	// コマンドリストの生成
	{
		hr = m_pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_pCmdAllocator[m_FrameIndex].Get(),
			nullptr,
			IID_PPV_ARGS(m_pCmdList.GetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}
	}

	// RTVの生成
	{
		// ディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		// ディスクリプタヒープの生成
		hr = m_pDevice->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(m_pHeapRTV.GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		for (auto i = 0; i < FrameCount; ++i)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pColorBuffer[i].GetAddressOf()));
			if (FAILED(hr))
			{
				return false;
			}
			// RTVの生成
			m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

			m_HandleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// フェンスの生成
	{
		// フェンスカウンターをリセット
		for (auto i = 0u; i < FrameCount; ++i)
		{
			m_FenceCounter[i] = 0;
		}

		// フェンスの生成
		hr = m_pDevice->CreateFence(
			m_FenceCounter[m_FrameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(m_pFence.GetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}

		m_FenceCounter[m_FrameIndex]++;

		// イベントの生成
		m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_FenceEvent == nullptr)
		{
			return false;
		}
	}

	// コマンドリストを閉じる
	m_pCmdList->Close();

	return true;
}

/// <summary>
/// Direct3Dの終了処理
/// </summary>
void Application::TermD3D()
{
	// GPU処理の完了を待機
	WaitGpu();

	// イベント破棄
	if (m_FenceEvent != nullptr)
	{
		CloseHandle(m_FenceEvent);
		m_FenceEvent = nullptr;
	}

	// フェンス破棄
	m_pFence.Reset();

	// RTVの破棄
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	// コマンドリストの破棄
	m_pCmdList.Reset();

	// コマンドアロケータの破棄
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pCmdAllocator[i].Reset();
	}

	// スワップチェーンの破棄
	m_pSwapChain.Reset();

	// コマンドキューの破棄
	m_pQueue.Reset();

	// デバイスの破棄
	m_pDevice.Reset();
}

/// <summary>
/// 描画処理
/// </summary>
void Application::Render()
{
	// コマンドの記録を開始
	m_pCmdAllocator[m_FrameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_FrameIndex].Get(), nullptr);

	// リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrior = {};
	barrior.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrior.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrior.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrior.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrior.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrior.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリア
	m_pCmdList->ResourceBarrier(1, &barrior);

	// レンダーターゲットの設定
	m_pCmdList->OMSetRenderTargets(1, &m_HandleRTV[m_FrameIndex], FALSE, nullptr);

	// クリアカラーの設定
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

	// RTVをクリア
	m_pCmdList->ClearRenderTargetView(m_HandleRTV[m_FrameIndex], clearColor, 0, nullptr);

	// 描画処理
	{
		// TODO
	}

	// リソースバリアの設定
	barrior.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrior.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrior.Transition.pResource = m_pColorBuffer[m_FrameIndex].Get();
	barrior.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrior.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrior.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリア
	m_pCmdList->ResourceBarrier(1, &barrior);

	// コマンドの記録を終了
	m_pCmdList->Close();

	// コマンドの実行
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// 画面に表示
	Present(1);
}

/// <summary>
/// GPU処理完了を待機します．
/// </summary>
void Application::WaitGpu()
{
	assert(m_pQueue.Get() != nullptr);
	assert(m_pFence.Get() != nullptr);
	assert(m_FenceEvent != nullptr);

	// シグナル処理
	m_pQueue->Signal(m_pFence.Get(), m_FenceCounter[m_FrameIndex]);

	// 完了時にイベントを設定する
	m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
	// 待機処理
	WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

	// 次のフレームのフェンスカウンターを増やす
	m_FenceCounter[m_FrameIndex]++;
}

/// <summary>
/// 画面に表示し、次のフレームの準備を行います．
/// </summary>
/// <param name="interval"> ディスプレイの垂直同期とフレームの表示を同期する方法 0 : 同期なし 1 : 同期あり</param>
void Application::Present(uint32_t interval)
{
	// 画面に表示
	m_pSwapChain->Present(interval, 0);

	// シグナル処理
	const auto currentValue = m_FenceCounter[m_FrameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);

	// バックバッファ番号を更新
	m_FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// 次のフレームの描画準備がまだであれば待機
	if (m_pFence->GetCompletedValue() < m_FenceCounter[m_FrameIndex])
	{
		m_pFence->SetEventOnCompletion(m_FenceCounter[m_FrameIndex], m_FenceEvent);
		WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
	}

	// 次のフレームのフェンスカウンターを増やす
	m_FenceCounter[m_FrameIndex] = currentValue + 1;
}

/// <summary>
/// ウィンドウプロシージャです．
/// </summary>
/// <param name="hWnd"> ウィンドウハンドル </param>
/// <param name="message"> メッセージ </param>
/// <param name="wParam"> 追加のメッセージ情報 </param>
/// <param name="lParam"> 追加のメッセージ情報 </param>
/// <returns></returns>
LRESULT Application::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0); // アプリケーションの終了を通知
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
