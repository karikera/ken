#pragma once

namespace kr
{
#ifdef WIN32
	struct COM
	{
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
	};
	struct OLE
	{
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
	};
	struct CommonControls
	{
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;
		};
	};
	struct GdiPlus
	{
		struct Init
		{
			Init() noexcept;
			~Init() noexcept;

			uintptr_t token;
		};
	};
#endif

	namespace _pri_
	{
		template <typename ... NEXTS>
		struct InitializeTails;

		template <typename TO, typename ... FROM>
		struct InitializeTailsReverse;

		template <typename T, bool isempty, typename NEXT>
		struct InitializeThis;

		template <typename T, typename NEXT>
		struct InitializeThis<T, true, NEXT>: NEXT
		{
			InitializeThis() noexcept
			{
				new((T*)this) T;
			}
			~InitializeThis() noexcept
			{
				((T*)this)->~T();
			}
		};

		template <typename T, typename NEXT>
		struct InitializeThis<T, false, NEXT> : NEXT
		{
			T _;

			InitializeThis() noexcept
			{
			}
			~InitializeThis() noexcept
			{
			}
		};

		template <typename NEXT, typename ... NEXTS>
		struct InitializeTails<NEXT, NEXTS ...> : InitializeThis<NEXT, std::is_empty<NEXT>::value, InitializeTails<NEXTS ...>>
		{
		};
		template <>
		struct InitializeTails<>
		{
		};
		template <typename ... TO, typename FROM, typename ... FROMS>
		struct InitializeTailsReverse<InitializeTails<TO...>, FROM, FROMS ...> : InitializeTailsReverse<InitializeTails<FROM, TO ...>, FROMS ...>
		{
		};
		template <typename ... TO>
		struct InitializeTailsReverse<InitializeTails<TO ...>>:InitializeTails<TO ... >
		{
		};
	}

	template <typename ... Inits>
	class Initializer: _pri_::InitializeTailsReverse<_pri_::InitializeTails<>, typename Inits::Init ...>
	{
	};
}

#ifdef WIN32

#ifdef _CONSOLE
#define main() __cdecl main() 
#else
#define main() __stdcall wWinMain(_In_ HINSTANCE__*,_In_opt_ HINSTANCE__*,_In_ wchar_t *,_In_ int)
#endif

#else

#define main() main() 

#endif
