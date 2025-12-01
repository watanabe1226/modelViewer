#pragma once
#include "pch.h"
#include "Graphics/RenderStage.h"

class Scene;

class ShadowStage : RenderStage
{
public:
	ShadowStage(Renderer* pRenderer);
	~ShadowStage();

	void Update(float deltaTime);
	void SetScene(Scene* newScene);

	void RecordStage(ID3D12GraphicsCommandList* pCmdList) override;

private:
	void CreateRootSignature(Renderer* pRenderer);
	void CreatePipeline(Renderer* pRenderer);

	Scene* m_pScene = nullptr;
};