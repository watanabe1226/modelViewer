#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <iostream>
#include <Windows.h>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
#define SMALL_NUMBER 1.e-8f

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")