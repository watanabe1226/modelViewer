#pragma once
#include "pch.h"

namespace DX12Utility
{
    class DX12Exception : public std::runtime_error
    {
    public:
        DX12Exception(const std::string& msg) : std::runtime_error(msg.c_str()) {}
    };

    inline void CheckResultCodeD3D12(HRESULT hr, const std::string errorMsg)
    {
        if (FAILED(hr))
            throw DX12Exception(errorMsg);
    }

    inline void CheckCodeD3D12(const std::string errorMsg)
    {
        throw DX12Exception(errorMsg);
    }

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }
}