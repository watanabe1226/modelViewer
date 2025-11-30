#include "Graphics/Transform.h"

Transform::Transform()
{
	World = Matrix4x4::Identity();
	View = Matrix4x4::Identity();
	Proj = Matrix4x4::Identity();
}
