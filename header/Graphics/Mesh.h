#pragma once
#include "pch.h"
#include "Graphics/Transform.h"

struct Vertex
{
	Vector3D position; // 頂点座標
	Vector4D color;    // 頂点カラー
};

class Mesh
{
public:
	Mesh();
	~Mesh();
};