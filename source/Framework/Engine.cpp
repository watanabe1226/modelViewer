#include "Framework/Engine.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/MathUtility.h"
#include "Utilities/Utility.h"

#include "Framework/Renderer.h"

namespace EngineInternal
{
	bool doResize = false;
}
using namespace EngineInternal;

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="width"> ウィンドウの横幅 </param>
/// <param name="height"> ウィンドウの縦幅 </param>
Engine::Engine(uint32_t width, uint32_t height)
{
	m_pRenderer = std::make_unique<Renderer>(width, height);
}

/// <summary>
/// デストラクタ
/// </summary>
Engine::~Engine()
{
}

/// <summary>
/// 実行します．
/// </summary>
void Engine::Run()
{
	MainLoop();
	TermApplication();
}

void Engine::RegisterWindowClass()
{
	// ウィンドウの設定
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX); // WNDCLASSEX構造体のサイズを設定
	wc.style = CS_HREDRAW | CS_VREDRAW; // ウィンドウのスタイルを設定
	wc.lpfnWndProc = WindowProc; // ウィンドウプロシージャの関数ポインタを設定
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // アプリケーションのアイコンを設定
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND); // 背景色を設定
	wc.lpszMenuName = nullptr; // メニュー名を設定しない
	wc.lpszClassName = TEXT("ModelViewerWindow"); // ウィンドウクラス名を設定
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION); // 小さいアイコンを設定

	// ウィンドウクラスの登録
	static auto atom = RegisterClassEx(&wc);
	assert(atom > 0);
}

/// <summary>
/// アプリ終了処理を行います．
/// </summary>
void Engine::TermApplication()
{
}

/// <summary>
/// メインループです.
/// </summary>
void Engine::MainLoop()
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
			Start();
			Update();
			Render();
		}
	}
}

void Engine::Start()
{
	if (doResize)
	{
		m_pRenderer->Resize();
		doResize = false;
	}
}

void Engine::Update()
{
}

void Engine::Render()
{
	m_pRenderer->Render();
}

/// <summary>
/// ウィンドウプロシージャです．
/// </summary>
/// <param name="hWnd"> ウィンドウハンドル </param>
/// <param name="message"> メッセージ </param>
/// <param name="wParam"> 追加のメッセージ情報 </param>
/// <param name="lParam"> 追加のメッセージ情報 </param>
/// <returns></returns>
LRESULT Engine::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0); // アプリケーションの終了を通知
			break;
		}
		case WM_SIZE:
		{
			doResize = true;
			break;
		}
		default:
		{
			break;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
