#pragma once
#include "pch.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"

const int MAX_AMOUNT_OF_LIGHTS = 15;

// Memory aligned lighting structs // 
struct PointLight
{
	Vector3D Position = Vector3D();	// 00 - 12 //
	float Intensity = 10.0f;		// 12 - 16 //
	Vector4D Color = Vector4D();	// 16 - 32 //
};

struct alignas(256) LightData
{
	PointLight pointLights[15]; // 000 - 480 //
	uint32_t activePointLights;	// 480 - 484 //
	Vector3D stub;				// 484 - 496 //
	Vector4D stub2;				// 496 - 512 // 
};