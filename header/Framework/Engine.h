#pragma once
#include "pch.h"
#include "Math/Matrix4x4.h"

class Renderer;

class Engine
{
public:
	Engine(uint32_t width, uint32_t height);
	~Engine();
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

	std::unique_ptr<Renderer> m_pRenderer = nullptr;
	
	bool Initialize();
	void TermApplication();
	bool InitWindow();
	void TermWindow();
	void MainLoop();
	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};