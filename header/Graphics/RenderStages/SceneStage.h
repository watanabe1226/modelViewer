#pragma once
#include "pch.h"
#include "Graphics/RenderStage.h"
#include "Graphics/Lights.h"

class Scene;
class Camera;
class ShadowStage;

class SceneStage : public RenderStage
{
public:
	SceneStage(Renderer* pRenderer, ShadowStage* pShadowStage);
	~SceneStage() override;
	void SetScene(Scene* newScene);

	void RecordStage(ID3D12GraphicsCommandList* pCmdList) override;

private:
	void CreateRootSignature(Renderer* pRenderer);
	void CreatePipeline(Renderer* pRenderer);

	Scene* m_pScene = nullptr;
	Camera* m_pCamera = nullptr;
	ShadowStage* m_pShadowStage = nullptr;
	ShadowLightData m_ShadowLightData;
};