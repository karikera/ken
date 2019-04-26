#pragma once

#include <KR3/main.h>

namespace kr
{
	class DefaultEscapeRule
	{
	public:
		template <typename O, typename C>
		static void parse(O * os, View<C> *text) noexcept
		{
		}
	};
	template <typename Encoder, typename EscapeRule = DefaultEscapeRule>
	class EscapeParser:public encoder::Parser<EscapeParser<Encoder>, typename Encoder::Component, typename Encoder::FromComponent>
	{
	public:
		using Component = typename Encoder::Component;
		using FromComponent = typename Encoder::FromComponent;

		template <typename _Parent, bool _accessable, typename _Parent>
		void parse(OutStream<_Parent, StreamInfo<Component, _accessable, _Parent>>* dest, kr::View<FromComponent> line) noexcept
		{
			kr::Text desc = line;
			while (!desc.empty())
			{
				char chr = *desc;
				if (chr != '\\')
				{
					desc = kr::meml<kr::Charset::Default>::next(desc);
					continue;
				}
				*dest << (Encoder)line.readto_p(desc);
				desc = kr::meml<kr::Charset::Default>::next(desc);
				if (desc.empty())
					return;

				chr = *desc;
				switch (chr)
				{
				case '/':
					*dest << (Component)'/';
					desc++;
					break;
				case '\\':
					*dest << (Component)'\\';
					desc++;
					break;
				case 't':
					*dest << (Component)'\t';
					desc++;
					break;
				case 'r':
					*dest << (Component)'\r';
					desc++;
					break;
				case 'n':
					*dest << (Component)'\n';
					desc++;
					break;
				default:
					EscapeRule::parse(os, &desc);
					break;
				}
			}
			*dest << (Encoder)line;
		}
	};
}
