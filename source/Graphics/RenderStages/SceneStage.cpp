#include "Graphics/RenderStages/SceneStage.h"
#include "Graphics/DX12RootSignature.h"
#include "Graphics/DX12PipelineState.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Framework/Renderer.h"
#include "Framework/Scene.h"
#include "Utilities/Utility.h"

SceneStage::SceneStage(Renderer* pRenderer) : RenderStage(pRenderer)
{
	CreateRootSignature(pRenderer);
	CreatePipeline(pRenderer);
}

SceneStage::~SceneStage()
{
}

void SceneStage::SetScene(Scene* newScene)
{
	m_pScene = newScene;
	m_pCamera = m_pScene->GetCamera();
}

void SceneStage::RecordStage(ID3D12GraphicsCommandList* pCmdList)
{
	if (m_pScene == nullptr)
	{
		assert(false && "シーンがnullです");
		return;
	}

	// リソースバリアの設定
	m_pRenderer->TransitionResource(m_pWindow->GetCurrentScreenBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->SetGraphicsRootSignature(m_pRootSignature->GetRootSignaturePtr());
	pCmdList->SetPipelineState(m_pPSO->GetPipelineStatePtr());

	// クリアカラーの設定
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	auto rtv = m_pWindow->GetCurrentScreenRTV();
	auto dsv = m_pWindow->GetDepthDSV();
	m_pRenderer->BindAndClearRenderTarget(m_pWindow, &rtv, &dsv, clearColor);

	auto view = m_pCamera->GetView();
	auto proj = m_pCamera->GetProj();
	for (const auto& model : m_pScene->GetModels())
	{
		model->Draw(view, proj);
	}

	// リソースバリアの設定
	m_pRenderer->TransitionResource(m_pWindow->GetCurrentScreenBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
}

void SceneStage::CreateRootSignature(Renderer* pRenderer)
{
	auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	// ルートパラメータの設定
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = 0; // t0
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメータ
	D3D12_ROOT_PARAMETER param[3] = {};

	// Transform CB : RootCBV
	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param[0].Descriptor.ShaderRegister = 0; // b0
	param[0].Descriptor.RegisterSpace = 0;
	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// Material CB : RootCBV
	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	param[1].DescriptorTable.NumDescriptorRanges = 1;  // b1
	param[1].Descriptor.RegisterSpace = 0;
	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// Texture Table : DescriptorTable
	param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[2].DescriptorTable.NumDescriptorRanges = 1;
	param[2].DescriptorTable.pDescriptorRanges = &range;
	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = -D3D12_FLOAT32_MAX;
	samplerDesc.MaxLOD = +D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = _countof(param);
	desc.NumStaticSamplers = 1;
	desc.pParameters = param;
	desc.pStaticSamplers = &samplerDesc;
	desc.Flags = flag;

	// ルートシグネチャの生成
	auto pDevice = m_pRenderer->GetDevice().Get();
	m_pRootSignature = std::make_unique<DX12RootSignature>(pDevice, &desc);
}

void SceneStage::CreatePipeline(Renderer* pRenderer)
{
	// 入力レイアウトの設定
	D3D12_INPUT_ELEMENT_DESC elements[4];
	elements[0].SemanticName = "POSITION";
	elements[0].SemanticIndex = 0;
	elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	elements[0].InputSlot = 0;
	elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[0].InstanceDataStepRate = 0;

	elements[1].SemanticName = "NORMAL";
	elements[1].SemanticIndex = 0;
	elements[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	elements[1].InputSlot = 0;
	elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[1].InstanceDataStepRate = 0;

	elements[2].SemanticName = "TEXCOORD";
	elements[2].SemanticIndex = 0;
	elements[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	elements[2].InputSlot = 0;
	elements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[2].InstanceDataStepRate = 0;

	elements[3].SemanticName = "TANGENT";
	elements[3].SemanticIndex = 0;
	elements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	elements[3].InputSlot = 0;
	elements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[3].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[3].InstanceDataStepRate = 0;

	// ラスタライザーステートの設定
	D3D12_RASTERIZER_DESC descRS;
	descRS.FillMode = D3D12_FILL_MODE_SOLID;
	descRS.CullMode = D3D12_CULL_MODE_NONE;
	descRS.FrontCounterClockwise = FALSE;
	descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	descRS.DepthClipEnable = FALSE;
	descRS.MultisampleEnable = FALSE;
	descRS.AntialiasedLineEnable = FALSE;
	descRS.ForcedSampleCount = 0;
	descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// レンダーターゲットのブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

	// ブレンドステートの設定
	D3D12_BLEND_DESC descBS;
	descBS.AlphaToCoverageEnable = FALSE;
	descBS.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		descBS.RenderTarget[i] = descRTBS;
	}

	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;

	// 頂点シェーダー読み込み
	static const std::wstring ShaderFilePathName = Utility::GetCurrentDir() + L"/assets/shaders/";
	auto hr = D3DReadFileToBlob((ShaderFilePathName + L"SimpleTexVS.cso").c_str(), vsBlob.GetAddressOf());
	ThrowFailed(hr);

	// ピクセルシェーダー読み込み
	hr = D3DReadFileToBlob((ShaderFilePathName + L"SimpleTexPS.cso").c_str(), psBlob.GetAddressOf());
	ThrowFailed(hr);

	// 深度ステンシルステートの設定
	D3D12_DEPTH_STENCIL_DESC descDSS = {};
	descDSS.DepthEnable = TRUE;
	descDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	descDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	descDSS.StencilEnable = FALSE;

	// パイプラインステートの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.InputLayout = { elements, _countof(elements) };
	desc.pRootSignature = m_pRootSignature->GetRootSignaturePtr();
	desc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
	desc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
	desc.RasterizerState = descRS;
	desc.BlendState = descBS;
	desc.DepthStencilState = descDSS;
	desc.SampleMask = UINT_MAX;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;

	// パイプラインステートの生成
	auto pDevice = m_pRenderer->GetDevice().Get();
	m_pPSO = std::make_unique<DX12PipelineState>(pDevice, &desc);
}
