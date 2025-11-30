#include "Graphics/Model.h"
#include "Graphics/Mesh.h"
#include "Utilities/Utility.h"
#include "Graphics/Window.h"
#include "Graphics/DX12DescriptorHeap.h"
#include "Graphics/DX12Commands.h"
#include "Framework/Renderer.h"
#include "Graphics/Texture.h"

Model::Model(Renderer* pRenderer, const std::wstring& filePath)
{
	m_pRenderer = pRenderer;
	if (filePath.c_str() == nullptr)
	{
		assert(false && "ファイル名がnullptrです");
		return;
	}

	// wchar_t から char への変換
	auto path = Utility::WStringToString(filePath);

	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;            // 三角形化
	flag |= aiProcess_PreTransformVertices;  // 変換の適用
	flag |= aiProcess_CalcTangentSpace;    // 接線空間の計算
	flag |= aiProcess_GenSmoothNormals;   // スムーズシェーディングの法線生成
	flag |= aiProcess_RemoveRedundantMaterials; // 冗長なマテリアルの削除
	flag |= aiProcess_OptimizeMeshes;       // メッシュの最適化
	flag |= aiProcess_MakeLeftHanded;      // 左手系に変換
	flag |= aiProcess_FlipUVs;              // UV反転

	// データの読み込み
	auto pScene = importer.ReadFile(path, flag);

	// チェック
	if (pScene == nullptr)
	{
		assert(false && "メッシュデータの読み込みに失敗しました");
		return;
	}

	auto numMat = pScene->mNumMaterials;
	m_Materials.shrink_to_fit();
	m_Materials.resize(numMat);
	for (auto i = 0; i < numMat; ++i)
	{
		PerseMaterial(pScene->mMaterials[i], m_Materials[i]);
	}

	auto numMeshes = pScene->mNumMeshes;
	m_pMeshes.shrink_to_fit();
	m_pMeshes.resize(numMeshes);
	for (auto i = 0u; i < numMeshes; ++i)
	{
		m_pMeshes[i] = std::make_unique<Mesh>(m_pRenderer, pScene->mMeshes[i]);
		auto materialIndex = m_pMeshes[i]->GetMaterialIndex();
		if (materialIndex != -1)
		{

			auto mat = m_Materials.at(materialIndex);
			m_pMeshes[i]->SetDiffuseTex(m_pRenderer->GetTexture(mat.m_DiffuseTexId));
		}
	}

	pScene = nullptr;

	m_Transform = Transform();

	m_pCommandList = m_pRenderer->GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetGraphicsCommandList().Get();
	m_pWindow = m_pRenderer->GetWindow();
	m_pCBVSRVUAVHeaps = m_pRenderer->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap();
}

Model::~Model()
{
}

void Model::Update(float deltaTime)
{
	//count += 0.01f;
	//m_Transform.World.setRotationY(count);
}

void Model::Draw(const Matrix4x4& viewMat, const Matrix4x4& projMat)
{
	if (m_pCommandList == nullptr)
	{
		assert(false && "コマンドリストがnullptrです");
		return;
	}
	auto backBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	for (auto i = 0; i < m_pMeshes.size(); ++i)
	{
		auto mesh = m_pMeshes[i].get();
		auto vbv = mesh->GetVBV();
		auto ibv = mesh->GetIBV();
		m_pCommandList->SetDescriptorHeaps(1, m_pCBVSRVUAVHeaps.GetAddressOf());
		m_Transform.View = viewMat;
		m_Transform.Proj = projMat;
		auto transformGPUAddress = m_pRenderer->AllocateConstantBuffer<Transform>(m_Transform, backBufferIndex);
		m_pCommandList->SetGraphicsRootConstantBufferView(0, transformGPUAddress);
		auto materialIndex = mesh->GetMaterialIndex();
		if (materialIndex != -1)
		{
			auto material = m_Materials[materialIndex];
			m_MaterialBuffer.Difuuse = material.m_Diffuse;
			m_MaterialBuffer.Alpha = material.m_Alpha;
			m_MaterialBuffer.Difuuse = material.m_Diffuse;
			m_MaterialBuffer.Difuuse = material.m_Diffuse;
		}
		else
		{
			m_MaterialBuffer.Difuuse = m_pRenderer->GetHalfVector3D();
			m_MaterialBuffer.Alpha = 1.0f;
			m_MaterialBuffer.Difuuse = m_pRenderer->GetHalfVector3D();
			m_MaterialBuffer.Difuuse = m_pRenderer->GetHalfVector3D();
		}
		auto matGPUAddress = m_pRenderer->AllocateConstantBuffer<MaterialBuffer>(m_MaterialBuffer, backBufferIndex);
		m_pCommandList->SetGraphicsRootConstantBufferView(1, matGPUAddress);
		m_pCommandList->SetGraphicsRootDescriptorTable(2, mesh->GetDiffuseTex()->GetSRV());

		m_pCommandList->IASetVertexBuffers(0, 1, &vbv);
		m_pCommandList->IASetIndexBuffer(&ibv);
		m_pCommandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
	}
}

Mesh* Model::GetMesh(uint32_t index)
{
	return m_pMeshes.at(index).get();
}

void Model::PerseMaterial(const aiMaterial* pSrcMat, Material& dstMat)
{
	aiColor3D color(0.f, 0.f, 0.f);

	// ディフューズ色の取得
	if (pSrcMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
	{
		dstMat.m_Diffuse = Vector3D(color.r, color.g, color.b);
	}
	else
	{
		dstMat.m_Diffuse = Vector3D(0.5f, 0.5f, 0.5f);
	}

	// スペキュラー色の取得
	if (pSrcMat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
	{
		dstMat.m_Specular = Vector3D(color.r, color.g, color.b);
	}
	else
	{
		dstMat.m_Specular = Vector3D(0.5f, 0.5f, 0.5f);
	}

	// 鏡面反射強度の取得
	auto shininess = 0.0f;
	if (pSrcMat->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS)
	{
		dstMat.m_Shininess = shininess;
	}
	else
	{
		dstMat.m_Shininess = 0.0f;
	}

	// ディフューズテクスチャの取得
	aiString texturePath;
	if (pSrcMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
	{
		auto path = Utility::StringToWString(texturePath.C_Str());
		m_pRenderer->CreateTextureFromFile(path.c_str());
		auto id = DX12Utility::StringHash(path.c_str());
		dstMat.m_DiffuseTexId = id;
	}
	else
	{
		dstMat.m_DiffuseTexId = -1;
	}
}
