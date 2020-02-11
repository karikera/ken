#pragma once

#include <KR3/main.h>
#include <KR3/meta/math.h>


namespace kr
{
	template <typename ... ARGS> class Mixed
	{
	private:
		alignas(meta::maxt(alignof(ARGS) ...)) struct {
			byte buffer[meta::maxt(sizeof(ARGS) ...)];
		} m_data;
		static constexpr byte EMPTY = 0xff;

		byte m_type;
		using types = meta::types<ARGS ...>;
		static_assert(sizeof ... (ARGS) < (1 << (sizeof(m_type)*8 - 1)), "");

		template <typename T, typename ... ARGS>
		T& _ctor(ARGS && ... args) noexcept
		{
			constexpr size_t index = types::template index_of<T>::value;
			static_assert(index != -1, "T is not allowed");

			T* now = (T*)&m_data;
			m_type = (byte)index;
			new(now) T(forward<ARGS>(args) ...);
			return *now;
		}

		void _dtor() noexcept
		{
			types::type_switch(m_type, [&](auto* t) {
				using T = remove_pointer_t<decltype(t)>;
				((T*)&m_data)->~T();
				});
		}

	public:
		Mixed() noexcept
		{
			m_type = EMPTY;
		}
		~Mixed() noexcept
		{
			if (m_type != EMPTY) _dtor();
		}

		bool empty() noexcept
		{
			return m_type == EMPTY;
		}

		void clear() noexcept
		{
			if (m_type == EMPTY) return;
			_dtor();
			m_type = EMPTY;
		}

		template <typename T>
		bool is() noexcept
		{
			constexpr size_t index = types::template index_of<T>::value;
			return m_type == index;
		}

		template <typename T>
		T& get() noexcept
		{
			_assert(is<T>());
			return *(T*)&m_data;
		}

		template <typename T, typename ... ARGS>
		T& reset(ARGS && ... args) noexcept
		{
			if (m_type != EMPTY) _dtor();
			return _ctor<T>(forward<ARGS>(args) ...);
		}

		template <typename LAMBDA>
		auto call(LAMBDA&& lambda) throws(...)->decltype((*(LAMBDA*)nullptr)(*(meta::typeAt<types, 0>*)nullptr))
		{
			if (m_type == EMPTY) return;
			types::type_switch(m_type, [&](auto* t) {
				using T = remove_pointer_t<decltype(t)>;
				lambda(*(T*)&m_data);
				});
		}
	};
}