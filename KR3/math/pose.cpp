#include "stdafx.h"
#include "pose.h"

using namespace kr;

const mat4a math::Pose::getMatrix() const noexcept
{
	mat4a rotate = orientation.toMatrix();
	rotate[0][3] = position.x;
	rotate[1][3] = position.y;
	rotate[2][3] = position.z;
	return rotate;
}
const mat4a math::Pose::getViewMatrix() const noexcept
{
	mat4a rotate	= orientation.toMatrix().transpose();
	rotate[0][3] = -dot(rotate[0], position);
	rotate[1][3] = -dot(rotate[1], position);
	rotate[2][3] = -dot(rotate[2], position);
	return rotate;
}
