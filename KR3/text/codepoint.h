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
		static void next(View<T>* text) noexcept
		{
			if (meml<charset>::isDbcs(**text))
			{
				(*text)++;
				if (text->empty()) return;
			}
			(*text)++;
		}

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

template <>
void kr::CodePoint<char, kr::Charset::Utf8>::next(kr::View<char>* text) noexcept;

template <>
void kr::CodePoint<char16_t, kr::Charset::Default>::next(kr::View<char16_t>* text) noexcept;
template <>
void kr::CodePoint<char32_t, kr::Charset::Default>::next(kr::View<char32_t>* text) noexcept;