#pragma once

#if _WIN_VER <= _WIN32_WINNT_WIN7
#error Unsupported windows version
#endif

namespace kr
{
	class XASound
	{
	public:

		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
	};
}
