#pragma once

#include <KR3/main.h>

namespace kr
{
	template <typename C>
	class StringStore
	{
		using AText = Array<C>;
		using Text = View<C>;
	private:
		AText m_buffer;
		Array<size_t> m_offset;

	public:
		StringStore() noexcept
		{
		}
		StringStore(const StringStore& _copy) noexcept
		{
			m_buffer = _copy.m_buffer;
			m_offset = _copy.m_offset;
		}
		size_t put(Text text) noexcept
		{
			size_t startAt = m_buffer.size();
			m_buffer << text;

			size_t idx = m_offset.size();
			m_offset.push(startAt);
			return idx;
		}
		size_t put(Text16 text) noexcept
		{
			size_t startAt = m_buffer.size();
			m_buffer << toUtf8(text);

			size_t idx = m_offset.size();
			m_offset.push(startAt);
			return idx;
		}

		Text get(size_t idx) const noexcept
		{
			_assert(idx < m_offset.size());
			const size_t* offset = m_offset.data() + idx;
			if (m_offset.size() == idx + 1)
			{
				return m_buffer.subarr(*offset);
			}
			else
			{
				return m_buffer.subarray(offset[0], offset[1]);
			}
		}
		void clear() noexcept
		{
			m_buffer = nullptr;
			m_offset = nullptr;
		}
	};
}
