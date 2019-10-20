#pragma once

#include <KR3/main.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/data/map.h>

namespace kr
{

	// 특정 태그를 특정 값으로 변환하는 출력 스트림
	// TOFIX: write 가 나눠서 될 경우, 나눠진 태그는 인식하지 못한다.
	template <typename Base, bool autoClose = false>
	class TemplateWriter:public io::FilterOStream<TemplateWriter<Base>, Base, autoClose>
	{
	public:
		static_assert(IsOStream<Base>::value, "Base is not OutStream");
		using Super = io::FilterOStream<TemplateWriter<Base>, Base, autoClose>;
		using Component = typename Base::Component;
		using Text = View<Component>;
		using Super::base;

		TemplateWriter(Base* _os, Text _open, Text _close) noexcept
			:Super(_os), m_open(_open), m_close(_close)
		{
		}
		TemplateWriter(typename Base::StreamableBase * _os, Text _open, Text _close) noexcept
			: Super(_os->template retype<Component>()), m_open(_open), m_close(_close)
		{
		}
		void put(Text key, Text value) noexcept
		{
			m_map[key] = value;
		}

		void writeImpl(const Component * data, size_t sz) noexcept
		{
			Text text(data, sz);
			for (;;)
			{
				Text next = text.find(m_open);
				if (next == nullptr)
				{
					*base() << text;
					return;
				}
				*base() << text.cut(next);
				next += 2;
				Text next2 = next.find(m_close);
				auto iter = m_map.find(next.cut(next2));
				if (iter != m_map.end())
					*base() << iter->second;
				text = next2 + 2;
			}
		}

	private:
		Text m_open, m_close;
		Map<Text, AText> m_map;
	};
}