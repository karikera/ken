#pragma once

#ifndef WIN32
#error is not windows system
#endif

#ifndef _WINDOWS_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../util/wide.h"
#include "../util/path.h"

namespace kr
{

	template <typename C>
	class ModuleName:public HasOnlyCopyTo<ModuleName<C>, C, HasOnlyCopyToInfo<Path::MAX_LEN, true> >
	{
	private:
		void* const m_module;

	public:
		ModuleName(const C* name = nullptr) noexcept;
		size_t $copyTo(C* dest) const noexcept;
	};

}

#endif