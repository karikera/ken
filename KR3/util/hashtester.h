#pragma once

#include "../main.h"

namespace kr
{
	template <typename T>
	class HashTester
	{
	private:
		size_t m_hash;
		size_t m_testHash;
		int m_hashoff;
		TmpArray<T> m_src;
		View<T> m_target;
		T * m_removeCursor;

	public:
		HashTester(View<T> target) noexcept
		{
			size_t tarhash = 0;
			{
				const T * t = target.begin();
				const T * te = target.end();
				while (t != te)
				{
					tarhash ^= *t;
					tarhash = intrinsic<sizeof(size_t)>::rotl(tarhash, 9);
					t++;
				}
			}
			m_hash = tarhash;
			m_testHash = 0;

			size_t tarsize = target.size();
			m_hashoff = (int)((9 * tarsize) & (sizeof(size_t) * 8 - 1));

			m_target = target;
			m_src.initResize(tarsize, '\0');
			m_removeCursor = m_src.begin();
			
		}

		bool put(T chr) noexcept
		{
			m_testHash ^= intrinsic<sizeof(size_t)>::rotl(*m_removeCursor, m_hashoff);
			m_testHash ^= chr;
			m_testHash = intrinsic<sizeof(size_t)>::rotl(m_testHash, 9);
			*m_removeCursor = chr;
			m_removeCursor++;
			if (m_removeCursor >= m_src.end()) m_removeCursor = m_src.begin();

			if (m_testHash == m_hash)
			{
				size_t cursorOffset = m_removeCursor - m_src.begin();
				if (cursorOffset == 0)
				{
					return memcmp(m_target.data(), m_src.data(), m_target.size() * sizeof(T)) == 0;
				}
				else
				{
					size_t cursorOffsetInv = m_src.end() - m_removeCursor;
					return memcmp(m_target.data(), m_removeCursor, cursorOffsetInv * sizeof(T)) == 0 &&
						memcmp(m_target.data() + cursorOffsetInv, m_src.data(), cursorOffset * sizeof(T)) == 0;
				}
			}
			return false;
		}
		const T * puts(View<T> src) noexcept
		{
			for (const T &chr : src)
			{
				if (put(chr)) return &chr;
			}
			return nullptr;
		}

		void next() noexcept
		{
		}
	};
}
