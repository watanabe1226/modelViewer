#pragma once
#include "Math/Matrix4x4.h"

class alignas(256) Transform
{
public:
	Transform();
	Matrix4x4 World; // ワールド変換行列
	Matrix4x4 View;  // ビュー変換行列
	Matrix4x4 Proj;  // プロジェクション変換行列

	// メモリ確保時に256バイト境界に合わせるカスタムnew
	void* operator new(size_t size)
	{
		return _aligned_malloc(size, 256);
	}

	// 配列版 new[]
	void* operator new[](size_t size)
	{
		return _aligned_malloc(size, 256);
	}

	// メモリ解放用のカスタムdelete
	void operator delete(void* ptr)
	{
		_aligned_free(ptr);
	}

	// 配列版 delete[]
	void operator delete[](void* ptr)
	{
		_aligned_free(ptr);
	}
};