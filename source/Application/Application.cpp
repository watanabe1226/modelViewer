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
	}
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
