#pragma once
#include "pch.h"
#include "Graphics/RenderStage.h"
#include "Graphics/Transform.h"

class Scene;
class Camera;
class Texture;
class Mesh;
class Model;

class SkydomeStage : public RenderStage
{
public:
	SkydomeStage(Renderer* pRenderer);
	~SkydomeStage();

	void RecordStage(ID3D12GraphicsCommandList* pCmdList) override;
	void SetScene(Scene* newScene);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSkydomeGPUHandle();
	Texture* GetHDRITex() { return m_pHDRITexture.get(); }

private:
	void CreateRootSignature(Renderer* pRenderer);
	void CreatePipeline(Renderer* pRenderer);

	Scene* m_pScene;
	Camera* m_pCamera = nullptr;
	std::unique_ptr<Texture> m_pHDRITexture = nullptr;
	std::unique_ptr<Texture> m_pTestTexture = nullptr;
	std::unique_ptr<Model> m_pSkydomeModel = nullptr;
	Mesh* m_pSkydomeMesh = nullptr;
	TransformBuffer m_SkydomeTranBuufer;

	
};