#pragma once

#include "../main.h"

namespace kr
{
	namespace _pri_
	{
		template <size_t value>
		using fit_uint_t = meta::if_t<
			(value <= (uint_8_t)-1), uint8_t,
			meta::if_t<
			(value <= (uint16_t)-1), uint16_t,
			meta::if_t<
			(value <= (uint32_t)-1), uint32_t,
			uint64_t
			> > >;
	}

	template <typename T, size_t SIZE>
	class StaticKeeper
	{
	private:
		static_assert(SIZE < sizeof(uint16_t), "");
		using IDX = _pri_::fit_uint_t<SIZE>;
		struct { byte buffer[sizeof(T)]; } alignas(alignof(T)) Buffer;

		Buffer buffers[SIZE];
		IDX unusing[SIZE];
		IDX unusingCount;
		ondebug(bool allocated[SIZE]);

	public:
		StaticKeeper() noexcept
		{
			for (size_t i = 0; i < SIZE; i++)
			{
				unusing[i] = (IDX)i;
			}
			unusingCount = (IDX)SIZE;
			ondebug(memset(allocated, 0, sizeof(allocated)));
		}
		T* allocWithoutCtor() throws(EofException)
		{
			if (unusingCount == 0) throw EofException();
			--unusingCount;
			size_t idx = unusing[unusingCount];
			ondebug(
				_assert(allocated[idx] == false);
			allocated[idx] = true;
			);
			return (T*)&buffers[idx];
		}

		template <typename ... ARGS>
		T* alloc(ARGS&& ... args) throws(...)
		{
			return new(allocWithoutCtor()) T(forward<ARGS>()...);
		}

		void free(T* freed) noexcept
		{
			size_t idx = (Buffer*)freed - buffers;
			_assert(idx < SIZE);
			unusing[usingCount++] = (IDX)idx;
			ondebug(
				_assert(allocated[idx] == true);
			allocated[idx] = false;
			);
		}
	};

}