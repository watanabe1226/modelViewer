#pragma once
#include "pch.h"
#include "Math/Matrix4x4.h"
#include "Graphics/Window.h"

class DX12Device;
class DX12RootSignature;
class DX12PipelineState;

class Renderer
{
public:
	Renderer(uint32_t width, uint32_t height);
	~Renderer();
	void Render();
	void Resize();

	struct alignas(256) Transform
	{
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
private:

	/// <summary>
	/// 頂点バッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pVB;
	/// <summary>
	/// インデックスバッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pIB;
	/// <summary>
	/// 定数バッファ
	/// </summary>
	ComPtr<ID3D12Resource> m_pCB[Window::FrameCount];

	uint32_t m_pCBVIndex[Window::FrameCount];

	Transform* m_Transforms[Window::FrameCount] = { nullptr };
	/// <summary>
	/// ルートシグネチャ
	/// </summary>
	std::unique_ptr<DX12RootSignature> m_pRootSignature = nullptr;
	/// <summary>
	/// パイプラインステート
	/// </summary>
	std::unique_ptr<DX12PipelineState> m_pPSO = nullptr;
	/// <summary>
	/// 頂点バッファビュー
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW m_VBV;
	/// <summary>
	/// インデックスバッファビュー
	/// </summary>
	D3D12_INDEX_BUFFER_VIEW m_IBV;
	/// <summary>
	/// 回転角
	/// </summary>
	float m_RotateAngle;
	uint32_t m_Width;
	uint32_t m_Height;

	bool OnInit();
};