#include "Graphics/Texture.h"
#include "Graphics/DX12Utilities.h"
#include "Graphics/DX12DescriptorHeap.h"
#include "Framework/Renderer.h"

namespace {

    //-----------------------------------------------------------------------------
    //      SRGBフォーマットに変換します.
    //-----------------------------------------------------------------------------
    DXGI_FORMAT ConvertToSRGB(DXGI_FORMAT format)
    {
        DXGI_FORMAT result = format;
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        { result = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_BC1_UNORM:
        { result = DXGI_FORMAT_BC1_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_BC2_UNORM:
        { result = DXGI_FORMAT_BC2_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_BC3_UNORM:
        { result = DXGI_FORMAT_BC3_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_B8G8R8A8_UNORM:
        { result = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_B8G8R8X8_UNORM:
        { result = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB; }
        break;

        case DXGI_FORMAT_BC7_UNORM:
        { result = DXGI_FORMAT_BC7_UNORM_SRGB; }
        break;

        default:
            break;
        }

        return result;
    }

}

Texture::Texture(Renderer* pRenderer, const std::wstring& filePath, D3D12_RESOURCE_FLAGS flag)
{
	auto pDevice = pRenderer->GetDevice().Get();
    DirectX::TexMetadata metaData = {};
    DirectX::ScratchImage image = {};
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;

    std::wstring fileName = ExChangeFileExtension(filePath);
    auto ext = FileExtension(fileName);
    HRESULT hr = S_FALSE;
    if (ext == L"png")
    {
        // テクスチャ生成
        hr = DirectX::LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_NONE, &metaData, image);
        ThrowFailed(hr);
    }
    else if (ext == L"tga")
    {
        hr = DirectX::LoadFromTGAFile(fileName.c_str(), &metaData, image);
        ThrowFailed(hr);
    }
    else if (ext == L"hdr")
    {
        hr = DirectX::LoadFromHDRFile(fileName.c_str(), &metaData, image);
        ThrowFailed(hr);
    }

    // アップロードヒープ用準備
    DirectX::PrepareUpload(pDevice, image.GetImages(), image.GetImageCount(), metaData, subResources);

    // CPUからアクセス可能なバッファの作成
    // ヒーププロパティ
    D3D12_HEAP_PROPERTIES textureProp = {};
    textureProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    textureProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    textureProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    textureProp.CreationNodeMask = 1;
    textureProp.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC desc = {};
    desc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
    desc.Format = metaData.format;
    desc.Width = static_cast<UINT>(metaData.width);
    desc.Height = static_cast<UINT>(metaData.height);
    desc.Flags = flag;
    desc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);

    pDevice->CreateCommittedResource(
        &textureProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(m_pResource.GetAddressOf())
    );

    const uint64_t texBufferSize = GetRequiredIntermediateSize(m_pResource.Get(), 0, 1);

    // ヒーププロパティ
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    // リソースの設定
    D3D12_RESOURCE_DESC uploadDesc = {};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Alignment = 0;
    uploadDesc.Width = texBufferSize;
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadDesc.SampleDesc.Count = 1;
    uploadDesc.SampleDesc.Quality = 0;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    uploadDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    pDevice->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(m_pUploadResource.GetAddressOf())
    );

    // コマンドの記録を開始
	auto pCommand = pRenderer->GetCommands(D3D12_COMMAND_LIST_TYPE_DIRECT);
    pCommand->ResetCommand();

    // テクスチャバッファの転送
	auto pCommandList = pCommand->GetGraphicsCommandList().Get();
    UpdateSubresources(pCommandList, m_pResource.Get(), m_pUploadResource.Get(), 0, 0, subResources.size(), subResources.data());

    // リソースバリア
    pRenderer->TransitionResource(m_pResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // コマンドの実行
    pCommand->ExecuteCommandList();
    // GPU処理の完了を待機
    pCommand->WaitGpu(INFINITY);

	// SRVの作成
	SRVHeap = pRenderer->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    srvIndex = SRVHeap->GetNextAvailableIndex();

    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = GetViewDesc(desc);

	pDevice->CreateShaderResourceView(
		m_pResource.Get(),
		&viewDesc,
		SRVHeap->GetCpuHandle(srvIndex)
	);
}

Texture::~Texture()
{
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetSRV() const
{
    return SRVHeap->GetGpuHandle(srvIndex);
}

D3D12_GPU_VIRTUAL_ADDRESS Texture::GetGPULocation() const
{
    return m_pResource->GetGPUVirtualAddress();
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetViewDesc(D3D12_RESOURCE_DESC desc)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};

    viewDesc.Format = desc.Format;
    viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (desc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_BUFFER:
    {
        assert("バッファは対象外");
    }
    break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
    {
        if (desc.DepthOrArraySize > 1)
        {
            viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;

            viewDesc.Texture1DArray.MostDetailedMip = 0;
            viewDesc.Texture1DArray.MipLevels = desc.MipLevels;
            viewDesc.Texture1DArray.FirstArraySlice = 0;
            viewDesc.Texture1DArray.ArraySize = desc.DepthOrArraySize;
            viewDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
        }
        else
        {
            viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;

            viewDesc.Texture1D.MostDetailedMip = 0;
            viewDesc.Texture1D.MipLevels = desc.MipLevels;
            viewDesc.Texture1D.ResourceMinLODClamp = 0.0f;
        }
    }
    break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
    {
        if (desc.DepthOrArraySize > 1)
        {
            if (desc.MipLevels > 1)
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;

                viewDesc.Texture2DMSArray.FirstArraySlice = 0;
                viewDesc.Texture2DMSArray.ArraySize = desc.DepthOrArraySize;
            }
            else
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;

                viewDesc.Texture2DArray.MostDetailedMip = 0;
                viewDesc.Texture2DArray.MipLevels = desc.MipLevels;
                viewDesc.Texture2DArray.FirstArraySlice = 0;
                viewDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
                viewDesc.Texture2DArray.PlaneSlice = 0;
                viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
            }
        }
        else
        {
            if (desc.MipLevels > 1)
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

                viewDesc.Texture2D.MostDetailedMip = 0;
                viewDesc.Texture2D.MipLevels = desc.MipLevels;
                viewDesc.Texture2D.PlaneSlice = 0;
                viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
            }
        }
    }
    break;

    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
    {
        viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;

        viewDesc.Texture3D.MostDetailedMip = 0;
        viewDesc.Texture3D.MipLevels = desc.MipLevels;
        viewDesc.Texture3D.ResourceMinLODClamp = 0.0f;
    }
    break;

    default:
    {
        // 想定外
        assert("想定外のテクスチャです.");
    }
    break;
    }

    return viewDesc;
}

std::wstring Texture::FileExtension(const std::wstring& filePath)
{
    auto idx = filePath.rfind(L'.');
    return filePath.substr(idx + 1, filePath.length() - idx - 1);
}

std::wstring Texture::ExChangeFileExtension(const std::wstring& filePath)
{
    auto idx = filePath.rfind(L'.');
    if (filePath.substr(idx + 1, filePath.length() - idx - 1) == L"psd")
    {
        return filePath.substr(0, idx) + L".tga";
    }
    else
        return filePath;
}
