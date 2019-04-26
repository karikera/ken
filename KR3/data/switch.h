#pragma once

#include "../main.h"
#include "../data/map.h"

namespace kr
{
	template <typename C> class TextSwitchT:public Container<C, true, Empty>
	{
	public:
		using Text = View<C>;

		inline TextSwitchT(initializer_list<Text> strs) noexcept
		{
			size_t counter = 0;
			for(Text str : strs)
			{
				m_map[str] = counter++;
			}
		}
		inline size_t operator [](Text key) const noexcept
		{
			auto res = m_map.find(key);
			if(res == m_map.end()) return -1;
			return res->second;
		}

	private:
		ReferenceMap<Text,size_t> m_map;
	};
		
	// enum
	using TextSwitch = TextSwitchT<char>;
	using TextSwitch16 = TextSwitchT<char16>;
	using TextSwitch32 = TextSwitchT<char32>;
}

#define  text_switcht(type,value, ...) \
	static const ::kr::TextSwitchT<type> CONCAT(__text_switch, __LINE__){__VA_ARGS__};\
	switch(CONCAT(__text_switch, __LINE__)[value])

#define  text_switch(value, ...) text_switcht(char, value, __VA_ARGS__)
#define  text_switch16(value, ...) text_switcht(::kr::char16, value, __VA_ARGS__)
#define  text_switch32(value, ...) text_switcht(::kr::char32, value, __VA_ARGS__)
