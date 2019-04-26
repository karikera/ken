#pragma once

#include "../meta/types.h"

namespace kr
{

	template <typename ... TYPES>
	class MixedArray
	{
		static_assert(sizeof ... (TYPES) <= maxof(dword), "Type count overflow");
		using types_t = meta::types<TYPES ...>;

		template <typename To, typename From>
		int _test_and_cast(dword code, To ** out, From * from) noexcept
		{
			if (code != types_t::index_of<From>::value) return 0;
			if (std::is_convertible<From*, To*>::value)
			{
				*out = (To*)from;
			}
			else _assert(!"Cannot convert");
			return 0;
		}
	public:
		MixedArray() noexcept
		{
			m_buffer = nullptr;
			m_size = 0;
		}
		~MixedArray() noexcept
		{
			clear();
		}
		void clear() noexcept
		{
			foreach([](auto * c) {
				callDestructor(c);
			});
			delete[] m_buffer;
			m_buffer = nullptr;
			m_size = 0;
		}

		template <typename TYPE>
		void add()
		{
			constexpr dword code = (dword)types_t::index_of<TYPE>::value;
			static_assert(code != -1, "No registered type");

			size_t sz = m_size;
			_resize(sz + alignedsize<TYPE, sizeof(dword)>::value + sizeof(dword));

			byte * dest = m_buffer + sz;
			*(dword*)(dest) = code;
			dest += sizeof(dword);
			_new((TYPE*)dest) TYPE();
		}
		template <typename LAMBDA>
		void foreach(LAMBDA lambda) noexcept
		{
			byte * src = m_buffer;
			byte * send = m_buffer + m_size;
			while (src != send)
			{
				dword code = *(dword*)src;
				src += sizeof(dword);

				int _dummy[] = {
					((code == types_t::index_of<TYPES>::value) ?
					(lambda((TYPES*)src), src += alignedsize<TYPES, sizeof(dword)>::value, 0) :
						0)
					...
				};
			}
		}

		template <typename TYPE>
		TYPE * get(size_t offset) noexcept
		{
			byte * src = m_buffer + offset;
			dword code = *(dword*)src;
			src += sizeof(dword);

			TYPE* out = nullptr;
			int _dummy[] = { _test_and_cast(code, &out, (TYPES*)src)... };
			return out;
		}

		size_t getOffsetOf(void * data) noexcept
		{
			return (((byte*)data) - m_buffer) - sizeof(dword);
		}

	private:
		void _resize(size_t nsize) noexcept
		{
			if (m_buffer == nullptr)
			{
				nbuffer = _new byte[nsize];
				m_size = nsize;
			}
			size_t cap = kr_msize(m_buffer);
			if (cap >= nsize)
			{
				m_size = nsize;
				return;
			}


			byte * nbuffer = _new byte[nsize];
			byte * dest = nbuffer;

			foreach([&](auto * src)
			{
				using TYPE = std::remove_pointer_t<decltype(src)>;
				dword code = (dword)types_t::index_of<TYPE>::value;
				*(dword*)dest = code;
				dest += sizeof(dword);
				_new((TYPE*)dest) TYPE(move(src));
				dest += alignedsize<TYPE, sizeof(dword)>::value;
			});

			m_buffer = nbuffer;
			m_size = nsize;
		}

		byte* m_buffer;
		size_t m_size;
	};
}
