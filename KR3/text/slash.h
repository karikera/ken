#pragma once

#include <KR3/main.h>
#include <KR3/meta/text.h>

namespace kr
{
	template <
		typename OS, typename OC, typename OInfo, 
		typename IS, typename IC, typename IInfo>
	void parseUnslashCharacter(
		OutStream<OS, OC, OInfo> * os, 
		InStream<IS, IC, IInfo> * is)
	{
		IC chr = is->read();
		switch (chr)
		{
		case 'a': os->write('\a'); break;
		case 'b': os->write('\b'); break;
		case 'f': os->write('\f'); break;
		case 'r': os->write('\r'); break;
		case 'n': os->write('\n'); break;
		case 't': os->write('\t'); break;
		case 'u':
			os->write((OC)((BArray<IC, 4>{
				is->read(),
				is->read(),
				is->read(),
				is->read()
			}).to_uint(16)));
			break;
		case 'x':
		{
			os->write((OC)((BArray<IC, 2>{
				is->read(),
				is->read()
			}).to_uint(16)));
			break;
		}
		case '0': os->write('\0'); break;
		case '\\':
		case '\'':
		case '\"':
		default: os->write(chr); break;
		}
	}

	template <typename C, typename LAMBDA>
	class CustomAddSlashes : public Printable<CustomAddSlashes<C, LAMBDA>, C>
	{
	private:
		const View<C> m_text;
		LAMBDA m_lambda;

	public:
		CustomAddSlashes(View<C> text, const LAMBDA & lambda) noexcept
			: m_text(text), m_lambda(lambda)
		{
		}
		CustomAddSlashes(View<C> text, LAMBDA&& lambda) noexcept
			: m_text(text), m_lambda(move(lambda))
		{
		}

		template <class _Derived, typename _Info>
		void $writeTo(OutStream<_Derived, C, _Info> * os) const
		{
			for (C s : m_text)
			{
				C rep = m_lambda(s);
				if (rep != (C)'\0')
				{
					*os << (C)'\\';
					*os << rep;
				}
				else
				{
					*os << s;
				}
			}
		}
	};

	template <typename C>
	class AddSlashes : public Printable<AddSlashes<C>, C>
	{
	private:
		const View<C> m_text;

	public:
		AddSlashes(View<C> text) noexcept
			: m_text(text)
		{
		}

		template <class _Derived, typename _Info>
		void $writeTo(OutStream<_Derived, C, _Info> * os) const
		{
			for (C s : m_text)
			{
				switch (s)
				{
				case (C)'\0': *os << (C)'\\'; *os << (C)'0'; break;
				case (C)'\r': *os << (C)'\\'; *os << (C)'r'; break;
				case (C)'\n': *os << (C)'\\'; *os << (C)'n'; break;
				case (C)'\t': *os << (C)'\\'; *os << (C)'t'; break;
				case (C)'\'': *os << (C)'\\'; *os << (C)'\''; break;
				case (C)'\"': *os << (C)'\\'; *os << (C)'\"'; break;
				case (C)'\\': *os << (C)'\\'; *os << (C)'\\'; break;
				default: *os << s; break;
				}
			}
		}
	};

	template <typename C>
	class StripSlashes : public Printable<StripSlashes<C>, C>
	{
	private:
		const View<C> m_text;

	public:
		StripSlashes(View<C> text) noexcept
			: m_text(text)
		{
		}

		template <class _Derived, typename _Info>
		void $writeTo(OutStream<_Derived, C, _Info> * os) const
		{
			View<C> text = m_text;
			while (!text.empty())
			{
				if (*text != '\\')
				{
					*os << *text++;
					continue;
				}
				text++;
				try
				{
					parseUnslashCharacter(os, &text);
				}
				catch (EofException&)
				{
					break;
				}
			}
		}
	};

	template <typename LAMBDA>
	inline auto addSlashesCustom(Text text, LAMBDA&& lambda) noexcept->CustomAddSlashes<char, decay_t<LAMBDA> >
	{
		return CustomAddSlashes<char, decay_t<LAMBDA> >(text, forward<LAMBDA>(lambda));
	}
	template <typename LAMBDA>
	inline auto addSlashesCustom(Text16 text, LAMBDA&& lambda) noexcept->CustomAddSlashes<char16, decay_t<LAMBDA> >
	{
		return CustomAddSlashes<char16, decay_t<LAMBDA> >(text, forward<LAMBDA>(lambda));
	}
	template <typename LAMBDA>
	inline auto addSlashesCustom(Text32 text, LAMBDA&& lambda) noexcept->CustomAddSlashes<char32, decay_t<LAMBDA> >
	{
		return CustomAddSlashes<char32, decay_t<LAMBDA> >(text, forward<LAMBDA>(lambda));
	}

	inline AddSlashes<char> addSlashes(Text text) noexcept { return text; }
	inline AddSlashes<char16> addSlashes(Text16 text) noexcept { return text; }
	inline AddSlashes<char32> addSlashes(Text32 text) noexcept { return text; }
	inline StripSlashes<char> stripSlashes(Text text) noexcept { return text; }
	inline StripSlashes<char16> stripSlashes(Text16 text) noexcept { return text; }
	inline StripSlashes<char32> stripSlashes(Text32 text) noexcept { return text; }
}

extern template class kr::AddSlashes<char>;
extern template class kr::StripSlashes<char>;
extern template class kr::AddSlashes<kr::char16>;
extern template class kr::StripSlashes<kr::char16>;
