#include "Graphics/RenderStages/SkydomeStage.h"
#include "Graphics/DX12RootSignature.h"
#include "Graphics/DX12PipelineState.h"
#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Graphics/Texture.h"
#include "Graphics/DX12DescriptorHeap.h""
#include "Graphics/Camera.h""
#include "Framework/Renderer.h"
#include "Framework/Scene.h"
#include "Utilities/Utility.h"

SkydomeStage::SkydomeStage(Renderer* pRenderer) : RenderStage(pRenderer)
{
	CreateRootSignature(pRenderer);
	CreatePipeline(pRenderer);

	std::wstring path = L"assets/models/Skydome/Skydome.gltf";
	m_pSkydomeModel = std::make_unique<Model>(pRenderer, path);
	m_pSkydomeMesh = m_pSkydomeModel->GetMesh(0);

	path = L"assets/HDRI/testDome.hdr";
	m_pHDRITexture = std::make_unique<Texture>(pRenderer, path, false, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	m_pTestTexture = std::make_unique<Texture>(pRenderer, path);
}

SkydomeStage::~SkydomeStage()
{
}

void SkydomeStage::RecordStage(ID3D12GraphicsCommandList* pCmdList)
{
	if (m_pScene == nullptr)
	{
		assert(false && "シーンがnullです");
		return;
	}
	auto backBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	auto cbvHeap = m_pRenderer->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pCmdList->SetGraphicsRootSignature(m_pRootSignature->GetRootSignaturePtr());
	pCmdList->SetPipelineState(m_pPSO->GetPipelineStatePtr());
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = cbvHeap->GetGpuHandle(m_pHDRITexture->GetSRVIndex());
	pCmdList->SetGraphicsRootDescriptorTable(0, gpuHandle);
	auto cameraPos = m_pCamera->GetPosition();
	auto cameraPosVec4D = Vector4D(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
	pCmdList->SetGraphicsRoot32BitConstants(1, 4, &cameraPosVec4D, 0);
	m_SkydomeTranBuufer.World = Matrix4x4::ScalingToMatrix(Vector3D(10000.0f, 10000.0f, 10000.0f));
	m_SkydomeTranBuufer.View = m_pCamera->GetView();
	m_SkydomeTranBuufer.Proj = m_pCamera->GetProj();
	pCmdList->SetGraphicsRoot32BitConstants(2, 48, &m_SkydomeTranBuufer, 0);

	auto vbv = m_pSkydomeMesh->GetVBV();
	auto ibv = m_pSkydomeMesh->GetIBV();
	pCmdList->IASetVertexBuffers(0, 1, &vbv);
	pCmdList->IASetIndexBuffer(&ibv);
	pCmdList->DrawIndexedInstanced(m_pSkydomeMesh->GetIndexCount(), 1, 0, 0, 0);
}

void SkydomeStage::SetScene(Scene* newScene)
{
	m_pScene = newScene;
	m_pCamera = m_pScene->GetCamera();
}

D3D12_GPU_DESCRIPTOR_HANDLE SkydomeStage::GetSkydomeGPUHandle()
{
	auto CBVHeap = m_pRenderer->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return CBVHeap->GetGpuHandle(m_pHDRITexture->GetSRVIndex());
}

void SkydomeStage::CreateRootSignature(Renderer* pRenderer)
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

	// hdri tex
	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[0].DescriptorTable.NumDescriptorRanges = 1;
	param[0].DescriptorTable.pDescriptorRanges = &range;
	param[0].Descriptor.RegisterSpace = 0;
	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// camera forward
	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	param[1].Constants.ShaderRegister = 0;  // b0
	param[1].Constants.RegisterSpace = 0;
	param[1].Constants.Num32BitValues = 4; // float4つ
	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// transform
	param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	param[2].Constants.ShaderRegister = 1;  // b1
	param[2].Constants.RegisterSpace = 0;
	param[2].Constants.Num32BitValues = 48; // matrix4x4 * 3
	param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// スタティックサンプラーの設定
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
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

void SkydomeStage::CreatePipeline(Renderer* pRenderer)
{
	// 入力レイアウトの設定
	D3D12_INPUT_ELEMENT_DESC elements[2];
	elements[0].SemanticName = "POSITION";
	elements[0].SemanticIndex = 0;
	elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	elements[0].InputSlot = 0;
	elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[0].InstanceDataStepRate = 0;

	elements[1].SemanticName = "TEXCOORD";
	elements[1].SemanticIndex = 0;
	elements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	elements[1].InputSlot = 0;
	elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	elements[1].InstanceDataStepRate = 0;

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
	auto hr = D3DReadFileToBlob((ShaderFilePathName + L"SkydomeVS.cso").c_str(), vsBlob.GetAddressOf());
	ThrowFailed(hr);

	// ピクセルシェーダー読み込み
	hr = D3DReadFileToBlob((ShaderFilePathName + L"SkydomePS.cso").c_str(), psBlob.GetAddressOf());
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
