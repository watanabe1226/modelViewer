#pragma once

#include <Windows.h>
#include <cstdint>

class Application
{
public:
	Application(uint32_t width, uint32_t height);
	~Application();
	void Run();

private:
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

	bool Initialize();
	void TermApplication();
	bool InitWindow();
	void TermWindow();
	void MainLoop();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};