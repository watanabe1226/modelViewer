#include "Framework/Scene.h"
#include "Graphics/Model.h"
#include "Graphics/Camera.h"
#include "Framework/Renderer.h"
#include "Graphics/Window.h"
#include "Utilities/Utility.h"

Scene::Scene(Renderer* pRenderer, uint32_t width, uint32_t height)
{
	m_pCamera = std::make_unique<Camera>(width, height);
	m_pRenderer = pRenderer;
	// レンダリングを開始する前にライトバッファが適切に更新されるようにするため
	// デフォルトではこれをtrueに設定
	m_IsEditedLight = true;

	std::string path = "assets/models/GroundPlane/plane.gltf";
	AddModel(path);
}

Scene::~Scene()
{
}

void Scene::Update(float deltaTime)
{
	m_SceneRuntime += deltaTime;
	m_pCamera->Update();

	if (m_IsEditedLight)
	{
		// ライトバッファの更新処理
		m_IsEditedLight = false;
	}

	for (const auto& model : m_pModels)
	{
		model->Update(deltaTime);
	}
}

void Scene::AddModel(const std::string filePath)
{
	m_pModels.push_back(std::make_unique<Model>(m_pRenderer, Utility::StringToWString(filePath)));
}

Camera* Scene::GetCamera() const
{
	return m_pCamera.get();
}

const std::vector<std::unique_ptr<Model>>& Scene::GetModels() const
{
	return m_pModels;
}

const LightData& Scene::GetLightData()
{
	return m_LightData;
}
