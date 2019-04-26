#pragma once

#include "fixed.h"

#ifdef WIN32

typedef struct _MAT2 MAT2;

#endif

namespace kr
{

	class fixedmatrix2
	{
	public:
		fixed _11;
		fixed _12;
		fixed _21;
		fixed _22;

		fixedmatrix2() noexcept;
		fixedmatrix2(fixed i11, fixed i12, fixed i21, fixed i22) noexcept;
		void Identity() noexcept;
		void SetRotation(float angle) noexcept;
		void SetScale(fixed sx, fixed sy) noexcept;
#ifdef WIN32
		explicit operator MAT2() noexcept;
#endif
	};

}
