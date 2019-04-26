#pragma once

#include "coord.h"

namespace kr
{
	namespace math
	{
		class Pose
		{
		public:
			const mat4a getMatrix() const noexcept;
			const mat4a getViewMatrix() const noexcept;

			vec4a		position;
			quaterniona	orientation;
		};
	}
}