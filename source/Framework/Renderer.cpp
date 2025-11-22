#include "Framework/Renderer.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/MathUtility.h"

#include "Utilities/Utility.h"
#include "Graphics/DX12Device.h"
#include "Graphics/DX12Commands.h"
#include "Graphics/Window.h"
#include "Graphics/DX12DescriptorHeap.h"
#include "Graphics/DX12RootSignature.h"
#include "Graphics/DX12PipelineState.h"

#include "Graphics/DX12Utilities.h"
#include "Graphics/DX12Access.h"

namespace RendererInternal
{
	std::unique_ptr<Window> window = nullptr;
	std::unique_ptr<DX12Device> device = nullptr;
	std::unique_ptr<DX12Commands> directCommands = nullptr;
	std::unique_ptr<DX12Commands> copyCommands = nullptr;

	std::unique_ptr<DX12DescriptorHeap> RTVHeap = nullptr;
	std::unique_ptr<DX12DescriptorHeap> DSVHeap = nullptr;
	std::unique_ptr<DX12DescriptorHeap> CBVHeap = nullptr;
}
using namespace RendererInternal;

struct Vertex
{
	Vector3D position; // 頂点座標
	Vector4D color;    // 頂点カラー
};

/// <summary>
/// コンストラクタ
/// </summary>
/// <param name="width"> ウィンドウの横幅 </param>
/// <param name="height"> ウィンドウの縦幅 </param>
Renderer::Renderer(uint32_t width, uint32_t height)
{
	// デバイスの生成
	device = std::make_unique<DX12Device>();
	// ディスクリプタヒープの生成
	RTVHeap = std::make_unique<DX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "RTVHeap", 16);
	DSVHeap = std::make_unique<DX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "DSVHeap", 10);
	CBVHeap = std::make_unique<DX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "CBVHeap", 5000, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	// コマンドの生成
	directCommands = std::make_unique<DX12Commands>(D3D12_COMMAND_LIST_TYPE_DIRECT);
	copyCommands = std::make_unique<DX12Commands>(D3D12_COMMAND_LIST_TYPE_COPY);

	// ウィンドウの作成
	window = std::make_unique<Window>(L"Model Viewer", width, height);
	m_Width = width;
	m_Height = height;
}

/// <summary>
/// 初期化時の処理です．
/// </summary>
bool Renderer::OnInit()
{
	ID3D12Device* pDevice = DX12Access::GetDevice().Get();
	// 頂点バッファの生成
	{
		// 頂点データ
		Vertex vertices[] =
		{
			Vector3D(-1.0f,  1.0f, 0.0f), Vector4D(1.0f, 0.0f, 0.0f, 1.0f),
			Vector3D(1.0f,  1.0f, 0.0f), Vector4D(0.0f, 1.0f, 0.0f, 1.0f),
			Vector3D(1.0f, -1.0f, 0.0f), Vector4D(0.0f, 0.0f, 1.0f, 1.0f),
			Vector3D(-1.0f, -1.0f, 0.0f), Vector4D(1.0f, 0.0f, 1.0f, 1.0f),
		};

		// ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込み可能なヒープ
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// リソースの設定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(vertices); // 頂点データのサイズ
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// リソースを生成
		auto hr = pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pVB.GetAddressOf())
		);

		// マッピング
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}
		// 頂点データをマッピング先に設定
		memcpy(ptr, vertices, sizeof(vertices));

		// マッピング解除
		m_pVB->Unmap(0, nullptr);

		// 頂点バッファビューの設定
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	// インデックスバッファの生成
	{
		uint32_t indices[] =
		{
			0, 1, 2,
			0, 2, 3,
		};

		// ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込み可能なヒープ
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// リソースの設定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(indices); // 頂点データのサイズ
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// リソースを生成
		auto hr = pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.GetAddressOf())
		);

		// マッピング
		void* ptr = nullptr;
		hr = m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			return false;
		}
		// 頂点データをマッピング先に設定
		memcpy(ptr, indices, sizeof(indices));

		// マッピング解除
		m_pIB->Unmap(0, nullptr);

		// 頂点バッファビューの設定
		m_IBV.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_IBV.Format = DXGI_FORMAT_R32_UINT;
		m_IBV.SizeInBytes = static_cast<UINT>(sizeof(indices));

	}

	// 定数バッファの生成
	{
		// ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込み可能なヒープ
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// リソースの設定
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(Transform);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (auto i = 0u; i < Window::FrameCount; ++i)
		{
			// リソース生成
			auto hr = pDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].GetAddressOf())
			);
			if (FAILED(hr))
			{
				return false;
			}

			m_pCBVIndex[i] = CBVHeap->GetNextAvailableIndex();
			m_Transforms[i] = std::make_unique<Transform>();
			// 定数バッファビューの設定
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
			desc.BufferLocation = m_pCB[i]->GetGPUVirtualAddress();
			desc.SizeInBytes = sizeof(Transform);
			// 定数バッファビューの生成
			pDevice->CreateConstantBufferView(&desc, CBVHeap->GetCpuHandle(m_pCBVIndex[i]));

			// マッピング
			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_Transforms[i]));
			if (FAILED(hr))
			{
				return false;
			}

			auto eyePos = Vector3D(0.0f, 0.0f, 5.0f);
			auto targetPos = Vector3D();
			auto upward = Vector3D(0.0f, 1.0f, 0.0f);

			auto fovY = MathUtility::DegreeToRadian(37.5f);
			auto aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);

			// 変換行列の設定
			m_Transforms[i]->World = Matrix4x4(); // 単位行列
			m_Transforms[i]->View = Matrix4x4::setLookAtLH(eyePos, targetPos, upward);
			m_Transforms[i]->Proj = Matrix4x4::setPerspectiveFovLH(fovY, aspect, 1.0f, 1000.0f);

		}
	}

	// ルートシグネチャ
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ルートパラメータ
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.Descriptor.ShaderRegister = 0;
		param.Descriptor.RegisterSpace = 0;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 1;
		desc.NumStaticSamplers = 0;
		desc.pParameters = &param;
		desc.pStaticSamplers = nullptr;
		desc.Flags = flag;

		// TODO : ルートシグネチャの生成
		m_pRootSignature = std::make_unique<DX12RootSignature>(&desc);
	}

	// パイプラインステートの生成
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

		elements[1].SemanticName = "COLOR";
		elements[1].SemanticIndex = 0;
		elements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
		auto hr = D3DReadFileToBlob((ShaderFilePathName + L"SimpleVS.cso").c_str(), vsBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		// ピクセルシェーダー読み込み
		hr = D3DReadFileToBlob((ShaderFilePathName + L"SimplePS.cso").c_str(), psBlob.GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

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
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		// パイプラインステートの生成
		hr = pDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pPSO->GetPipelineState().GetAddressOf()));
		if (FAILED(hr))
		{
			return false;
		}
	}
	return true;
}

/// <summary>
/// 描画処理
/// </summary>
void Renderer::Render()
{
	uint32_t backBufferIndex = window->GetCurrentBackBufferIndex();
	// コマンドの記録を開始とリセット
	directCommands->ResetCommand(backBufferIndex);
	// リソースバリアの設定
	DX12Utility::TransitionResource(window->GetCurrentScreenBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	ID3D12GraphicsCommandList* pCmdList = DX12Access::GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetGraphicsCommandList().Get();

	// クリアカラーの設定
	float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	auto rtv = window->GetCurrentScreenRTV();
	auto dsv = window->GetCurrentScreenRTV();
	DX12Utility::BindAndClearRenderTarget(window.get(),
		&rtv,
		&dsv,
		clearColor);
	// 描画処理
	{
		pCmdList->SetGraphicsRootSignature(m_pRootSignature->GetRootSignaturePtr());
		pCmdList->SetDescriptorHeaps(1, CBVHeap->GetHeap().GetAddressOf());
		pCmdList->SetGraphicsRootConstantBufferView(0, m_pCB[backBufferIndex]->GetGPUVirtualAddress());
		pCmdList->SetPipelineState(m_pPSO->GetPipelineStatePtr());

		pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		pCmdList->IASetIndexBuffer(&m_IBV);

		pCmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	// リソースバリアの設定
	DX12Utility::TransitionResource(window->GetCurrentScreenBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	// コマンドリストの実行
	directCommands->ExecuteCommandList(backBufferIndex);

	// 画面に表示
	window->Present(1);
}

void Renderer::Resize()
{
	uint32_t backBufferIndex = window->GetCurrentBackBufferIndex();
	directCommands->WaitGpu(INFINITE, backBufferIndex);
	window->Resize();
}

#pragma region DX12Access Implementations
ComPtr<ID3D12Device> DX12Access::GetDevice()
{
	if (!device)
	{
		assert(false && "DXDevice hasn't been initialized yet, call will return nullptr");
	}

	return device.get();
}

DX12Commands* DX12Access::GetCommands(D3D12_COMMAND_LIST_TYPE type)
{
	if (!directCommands || !copyCommands)
	{
		assert(false && "Commands haven't been initialized yet, call will return nullptr");
	}

	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return copyCommands.get();
		break;

	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return directCommands.get();
		break;
	}

	assert(false && "This command type hasn't been added yet!");
	return nullptr;
}

DX12DescriptorHeap* DX12Access::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	switch (type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return CBVHeap.get();
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVHeap.get();
		break;

	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVHeap.get();
		break;
	}

	// Invalid type passed
	assert(false && "Descriptor type passed isn't a valid or created type!");
	return nullptr;
}

Window* DX12Access::GetWindow()
{
	return window.get();
}
#pragma endregion
