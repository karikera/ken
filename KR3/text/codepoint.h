#pragma once

#include <KR3/main.h>

namespace kr
{
	template <typename T, Charset charset = Charset::Default>
	class CodePoint
	{
	private:
		View<T> m_text;

	public:
		static void next(View<T> * text) noexcept;

		class Iterator
		{
		private:
			typename View<T>::InternalComponentRef * m_begin;
			View<T> m_iter;

		public:
			Iterator(View<T> text) noexcept
				:m_iter(text)
			{
				m_begin = text.begin();
				if (!m_iter.empty()) next(&m_iter);
			}
			bool isEnd() const noexcept
			{
				return m_begin == m_iter.begin();
			}
			View<T> operator *() const noexcept
			{
				return View<T>(m_begin, m_iter.begin());
			}
			Iterator & operator ++() noexcept
			{
				m_begin = m_iter.begin();
				if (!m_iter.empty()) next(&m_iter);
				return *this;
			}
			const Iterator operator ++(int) noexcept
			{
				Iterator old = *this;
				++*this;
				return old;
			}
		};

		CodePoint(View<T> text) noexcept
			:m_text(text)
		{
		}
		Iterator begin() noexcept
		{
			return m_text;
		}
		IteratorEnd end() noexcept
		{
			return IteratorEnd();
		}
	};

	template <typename T>
	CodePoint<T> codePoint(View<T> text) noexcept
	{
		return CodePoint<T>(text);
	}
}