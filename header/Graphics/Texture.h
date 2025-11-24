#pragma once
#include "pch.h"

class Texture
{
public:
	Texture(const std::string& filePath);
	~Texture();

	uint32_t GetSRVIndex() const { return srvIndex; }
	ComPtr<ID3D12Resource> GetResource() const { return m_pResource; }
	ID3D12Resource* GetResourcePtr() const { return m_pResource.Get(); }

private:
	ComPtr<ID3D12Resource> m_pResource = nullptr;
	uint32_t srvIndex = 0;
};