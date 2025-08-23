#include <windows.h>
#include "Application/Application.h"

/// <summary>
/// エントリーポイントです．
/// </summary>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR pCmdLine, int nCmdShow)
{
	// ウィンドウのサイズを指定
	Application app(960, 540);
	app.Run();

	return 0;
}