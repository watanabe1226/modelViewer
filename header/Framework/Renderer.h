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
	void Render();
	void Resize();

	struct alignas(256) Transform
	{
		Matrix4x4 World; // ワールド変換行列
		Matrix4x4 View;  // ビュー変換行列
		Matrix4x4 Proj;  // プロジェクション変換行列
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

	std::unique_ptr<Transform> m_Transforms[Window::FrameCount];
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