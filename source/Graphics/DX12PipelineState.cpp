#include "Graphics/DX12PipelineState.h"
#include "Graphics/DX12Access.h"
#include "Graphics/DX12Utilities.h"

DX12PipelineState::DX12PipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc)
{
	// パイプラインステートの生成
	auto hr = DX12Access::GetDevice().Get()->CreateGraphicsPipelineState(
		pDesc,
		IID_PPV_ARGS(m_pPipelineState.GetAddressOf()));
	ThrowFailed(hr);
}

DX12PipelineState::DX12PipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc)
{
	// パイプラインステートの生成
	auto hr = DX12Access::GetDevice().Get()->CreateComputePipelineState(
		pDesc,
		IID_PPV_ARGS(m_pPipelineState.GetAddressOf()));
	ThrowFailed(hr);
}
