#pragma once
#include "pch.h"
#include "Graphics/Transform.h"
#include "Graphics/DX12Utilities.h"
#include "Graphics/Materials.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh;
class Window;
class Renderer;

class Model
{
public:
	Model(Renderer* pRenderer, const std::wstring& filePath);
	Model(const Model& model) = delete;
	Model& operator=(const Model& model) = delete;
	~Model();
	void Update(float deltaTime);

	void Draw(const Matrix4x4& viewMat, const Matrix4x4& projMat);

	Mesh* GetMesh(uint32_t index);

	Transform m_Transform;
	MaterialBuffer m_MaterialBuffer;
	std::string m_Name;

private:
	void PerseMaterial(const aiMaterial* pSrcMat, Material& dstMat);

	std::vector<std::unique_ptr<Mesh>> m_pMeshes;
	std::vector<Material> m_Materials;

	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
	Window* m_pWindow = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_pCBVSRVUAVHeaps = nullptr;
	Renderer* m_pRenderer = nullptr;

	float count = 0.f;
};