#include "pch.h"
#include "Application/Application.h"


/// <summary>
/// エントリーポイントです．
/// </summary>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// ウィンドウのサイズを指定
	Application app(960, 540);
	app.Run();

	return 0;
}