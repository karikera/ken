#pragma once

#include "../main.h"

namespace kr
{
	template <typename C>
	class HashTester
	{
	private:
		using InternalComponent = internal_component_t<C>;

		size_t m_hash;
		size_t m_testHash;
		int m_hashoff;
		TmpArray<C> m_src;
		const View<C> m_needle;
		InternalComponent * m_removeCursor;

	public:

		class NoReset_t final {};
		static constexpr const NoReset_t NoReset = NoReset_t();

#pragma warning(push)
#pragma warning(disable:26495)
		HashTester(View<C> needle, NoReset_t) noexcept
			:m_needle(needle)
		{
			m_src.resize(needle.size());
			_assert(needle != nullptr && !needle.empty());
			size_t tarhash = 0;
			{
				const InternalComponent* t = needle.begin();
				const InternalComponent* te = needle.end();
				while (t != te)
				{
					tarhash ^= *t;
					tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
					t++;
				}
			}
			m_hash = tarhash;
		}
#pragma warning(pop)

		HashTester(View<C> needle) noexcept
			:HashTester(needle, NoReset)
		{
			reset();
		}

		void reset() noexcept
		{
			m_testHash = 0;

			size_t tarsize = m_needle.size();
			m_hashoff = (int)((9 * tarsize) & (sizeof(size_t) * 8 - 1));

			m_src.fill('\0');
			m_removeCursor = m_src.begin();
		}
		bool put(InternalComponent data) noexcept
		{
			m_testHash ^= intrinsic<sizeof(size_t)>::rotl(*m_removeCursor, m_hashoff);
			m_testHash ^= data;
			m_testHash = intrinsic<sizeof(size_t)>::rotl(m_testHash, 9);
			*m_removeCursor = data;
			m_removeCursor++;
			if (m_removeCursor >= m_src.end()) m_removeCursor = m_src.begin();

			if (m_testHash == m_hash)
			{
				size_t cursorOffset = m_removeCursor - m_src.begin();
				if (cursorOffset == 0)
				{
					return memcmp(m_needle.data(), m_src.data(), m_needle.size() * sizeof(InternalComponent)) == 0;
				}
				else
				{
					size_t cursorOffsetInv = m_src.end() - m_removeCursor;
					return memcmp(m_needle.data(), m_removeCursor, cursorOffsetInv * sizeof(InternalComponent)) == 0 &&
						memcmp((InternalComponent*)m_needle.data() + cursorOffsetInv, m_src.data(), cursorOffset * sizeof(InternalComponent)) == 0;
				}
			}
			return false;
		}
		const C * puts(View<C> data) noexcept
		{
			for (const InternalComponent &chr : data)
			{
				if (put(chr)) return &chr+1;
			}
			return nullptr;
		}

		size_t size() const noexcept
		{
			return m_needle.size();
		}

	};
}
