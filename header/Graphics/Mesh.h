#pragma once
#include "pch.h"
#include "Math/Vector2D.h"
#include "Math/Vector3D.h"
#include "Graphics/DX12Utilities.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex
{
	Vector3D m_Position; // 頂点座標
	Vector3D m_Normal;    // 法線ベクトル
	Vector2D m_TexCoord;    // UV座標
	Vector3D m_Tangent;    // 接線ベクトル
};

class Texture;

class Mesh
{
public:
	Mesh(Renderer* pRenderer, const aiMesh* pSrcMesh);
	~Mesh();
	D3D12_VERTEX_BUFFER_VIEW GetVBV() const { return m_VBV; }
	D3D12_INDEX_BUFFER_VIEW GetIBV() const { return m_IBV; }
	uint32_t GetIndexCount() const { return m_IndexCount; }
	uint32_t GetMaterialIndex() const { return m_MaterialIndex; }
	void SetMaterialIndex(uint32_t index) { m_MaterialIndex = index; }
	void SetDiffuseTex(Texture* pTexture) { m_pDiffuseTexture = pTexture; }
	Texture* GetDiffuseTex() const { return m_pDiffuseTexture; }
	std::string m_Name;

private:
	void UploadBuffers(ID3D12Device* pDevice);

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	uint32_t m_MaterialIndex = -1;

	Texture* m_pDiffuseTexture = nullptr;

	// 頂点、インデックスデータ
	ComPtr<ID3D12Resource> m_pVB = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VBV = {};

	ComPtr<ID3D12Resource> m_pIB = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_IBV = {};

	uint32_t m_IndexCount = 0;
	Renderer* m_pRenderer = nullptr;
};