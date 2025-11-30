#pragma once
#include "pch.h"
#include "Graphics/RenderStage.h"

class Scene;
class Camera;

class SceneStage : public RenderStage
{
public:
	SceneStage(Renderer* pRenderer);
	~SceneStage() override;
	void SetScene(Scene* newScene);

	void RecordStage(ID3D12GraphicsCommandList* pCmdList) override;

private:
	void CreateRootSignature(Renderer* pRenderer);
	void CreatePipeline(Renderer* pRenderer);

	Scene* m_pScene = nullptr;
	Camera* m_pCamera = nullptr;
};