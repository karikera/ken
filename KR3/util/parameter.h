#pragma once

#include <KR3/main.h>
#include <KR3/data/map.h>

namespace kr
{
	class PrefixMap
	{
	public:
		static constexpr size_t ONEMAP_SIZE = ('Z' - 'A' + 1) + ('z' - 'a' + 1);

		PrefixMap() noexcept;

		template <typename C, size_t sz>
		void addText(const C(&text)[sz]) throws(InvalidSourceException, DuplicateException);

		template <typename C>
		void addText(View<C> text) throws(InvalidSourceException, DuplicateException);

		template <typename C, size_t sz>
		View<C> find(const C(&text)[sz]) throws(NotFoundException);

		template <typename C>
		View<C> find(View<C> text) throws(NotFoundException);

		template <typename C>
		static size_t toIndex(C chr) throws(NotFoundException);

	private:
		Array<size_t> m_array;
	};

	template <typename C, size_t sz>
	void PrefixMap::addText(const C(&text)[sz]) throws(InvalidSourceException, DuplicateException)
	{
		return addText((View<C>)text);
	}
	template <typename C>
	void PrefixMap::addText(View<C> text) throws(InvalidSourceException, DuplicateException)
	{
		if (text.empty())
			throw InvalidSourceException();

		size_t * array = m_array.data();
		for (;;)
		{
			size_t & next = array[toIndex(*text)];
			if (next == (size_t)-1)
				throw DuplicateException();

			text++;

			if (next != 0)
			{
				if (text.empty())
					throw DuplicateException();
				array = m_array.data() + next;
			}
			else
			{
				if (text.empty())
				{
					next = (size_t)-1;
					return;
				}

				next = m_array.size();
				array = m_array.prepare(ONEMAP_SIZE);
				mema::zero(array, ONEMAP_SIZE);
			}
		}
	}
	template <typename C, size_t sz>
	View<C> PrefixMap::find(const C(&text)[sz]) throws(NotFoundException)
	{
		return find(View<C>(text));
	}
	template <typename C>
	View<C> PrefixMap::find(View<C> text) throws(NotFoundException)
	{
		size_t * array = m_array.data();
		for (;;)
		{
			if (text.empty())
				throw EofException();

			size_t next = array[toIndex(*text)];
			if (next == (size_t)-1)
				return text + 1;
			if (next == 0)
				throw NotFoundException();
			array = m_array.data() + next;
			text++;
		}
	}
	template <typename C>
	size_t PrefixMap::toIndex(C chr) throws(NotFoundException)
	{
		if (chr > (C)'Z')
		{
			if (chr < (C)'a')
				throw NotFoundException();
			if (chr >= (C)'z')
				throw NotFoundException();
			return ((size_t)chr - (size_t)'a') + (size_t)('Z' - 'A' + 1);
		}
		else
		{
			if (chr < (C)'A')
				throw NotFoundException();
			return (size_t)chr - (size_t)'A';
		}
	}

	template <typename C> class ParameterTokenizerT
	{
	public:
		using Text = View<C>;
		ParameterTokenizerT(const C * params) noexcept;

		template <typename LAMBDA> void remaining(LAMBDA&& lambda) throws(NotFoundException);
		Array<C> nextString() throws(NotFoundException);
		int nextInt() throws(NotFoundException);
		bool hasNext() noexcept;

	private:
		const C * m_params;
	};

	enum ParamType
	{
		ParamNoValue,
		ParamPrefix,
		ParamValue
	};
	template <typename C> 
	template <typename LAMBDA>
	void ParameterTokenizerT<C>::remaining(LAMBDA&& lambda) throws(NotFoundException)
	{
		if(m_params == nullptr) throw NotFoundException();
		const wchar_t * i = m_params;
		for(;;)
		{
			wchar_t chr = *i++;
			if(chr == '\0') break;
			lambda(chr);
		}
		m_params = nullptr;
	}

	template <typename Component>
	class ParameterT
	{
	public:
		using Text = View<Component>;
		using AText = Array<Component>;

		struct ParamInfo
		{
			ParamType type;
			Text name;
			char shortCut;

			inline ParamInfo(ParamType type, Text name, char shortCut=0) noexcept
				:type(type), name(name), shortCut(shortCut)
			{
			}
		};

		struct Value
		{
			Text name;
			Text value;
		};
		ParameterT(std::initializer_list<ParamInfo> paramNames) noexcept;
		~ParameterT() noexcept;

		void start(int argn, Component ** args) noexcept;

		Value next() throws(EofException, InvalidSourceException);

		template <typename LAMBDA>
		bool foreach(LAMBDA && lambda) noexcept
		{
			try
			{
				for (;;)
				{
					Value value = next();
					lambda(value.name, value.value);
				}
			}
			catch (EofException&)
			{
				return true;
			}
			catch (...)
			{
				return false;
			}
		}

		void printPosition() noexcept;

	private:
		View<Component> _getValue(Text name) throws(EofException, InvalidSourceException);
		View<Component> _getValue(const ParamInfo &info) throws(EofException, InvalidSourceException);
		View<Component> _get() throws(EofException);
		void _checkName(Text name) throws(InvalidSourceException);
		View<Component> _checkShortCut(Component shortCut) throws(InvalidSourceException);

		PrefixMap m_prefix;
		Map<Text, ParamInfo> m_map;
		Text m_shortCut[PrefixMap::ONEMAP_SIZE];
		Component ** m_begin;
		Component ** m_iter;
		Component ** m_end;
	};

	TText readArgument(Text* line) noexcept;
	TText16 readArgument(Text16* line) noexcept;

	TText readArgument(char** text) noexcept;
	TText16 readArgument(char16** text) noexcept;

	EXTERN_FULL_CHAR_CLASS(ParameterTokenizer);
	EXTERN_FULL_CHAR_CLASS(Parameter);
}
