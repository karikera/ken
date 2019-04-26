#pragma once

namespace kr
{

	template <typename T, typename... ARGS> T* constructor(ARGS... args)
	{
		return _new T(args...);
	}

}