#include "Graphics/DX12RootSignature.h"
#include "Graphics/DX12Access.h"
#include "Graphics/DX12Utilities.h"

DX12RootSignature::DX12RootSignature(const D3D12_ROOT_SIGNATURE_DESC* pDesc)
{
	ComPtr<ID3DBlob> pBlob = nullptr;
	ComPtr<ID3DBlob> pErrorBlob = nullptr;

	// シリアライズ
	auto hr = D3D12SerializeRootSignature(
		pDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		pBlob.GetAddressOf(),
		pErrorBlob.GetAddressOf()
	);
	ThrowFailed(hr);

	// ルートシグネチャの生成
	hr = DX12Access::GetDevice().Get()->CreateRootSignature(
		0,
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		IID_PPV_ARGS(m_pRootSignature.GetAddressOf())
	);
	ThrowFailed(hr);
}
