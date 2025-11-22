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

	std::unique_ptr<Renderer> m_pRenderer = nullptr;
	
	void RegisterWindowClass();
	void TermApplication();
	void MainLoop();
	void Start();
	void Update();
	void Render();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};