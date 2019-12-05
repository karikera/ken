#pragma once

#include <KR3/main.h>
#include <KR3/data/switch.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/meta/function.h>
#include "parser.h"

namespace kr
{
	template <typename T>
	struct JsonParsable;
	class JsonParser;
	class JsonField;
	class JsonArray;

	template <typename T>
	struct JsonFieldCaller;

	template <>
	struct JsonFieldCaller<JsonField>
	{
		template <typename LAMBDA>
		static void call(JsonParser * parser, LAMBDA && lambda) throws(InvalidSourceException);
	};

	template <>
	struct JsonFieldCaller<JsonArray>
	{
		template <typename LAMBDA>
		static void call(JsonParser * parser, LAMBDA && lambda) throws(InvalidSourceException);
	};

	class JsonParser: private Parser
	{
		friend JsonFieldCaller<JsonField>;
		friend JsonFieldCaller<JsonArray>;
	public:
		enum Type
		{
			Null,
			Number,
			String,
			Array,
			Boolean,
			Object
		};

		JsonParser(io::VIStream<char> is) noexcept;
		~JsonParser() noexcept;
		using Parser::getLine;
		Type getNextType() throws(InvalidSourceException);
		float floatNumber() throws(InvalidSourceException);
		int integer() throws(InvalidSourceException);
		int64_t integer64() throws(InvalidSourceException);
		uint uinteger() throws(InvalidSourceException);
		uint64_t uinteger64() throws(InvalidSourceException);
		bool boolean() throws(InvalidSourceException);
		AText16 text16() throws(InvalidSourceException);
		AText text() throws(InvalidSourceException);
		TText ttext() throws(InvalidSourceException);
		void skipValue() throws(InvalidSourceException);

		template <typename LAMBDA>
		void array(LAMBDA && lambda) throws(InvalidSourceException)
		{
			if (_skipIfNot('[')) return;
			char oldchr = _open(']');			
			size_t idx = 0;
			while (m_closeCharacter != 0)
			{
				lambda(idx++);
			}
			m_closeCharacter = oldchr;
			_closeTest();
		}

		template <typename T>
		T read() throws(InvalidSourceException)
		{
			T value;
			read(&value);
			return value;
		}
		template <typename T>
		void read(T * value) throws(InvalidSourceException)
		{
			JsonParsable<T>::parse(*this, value);
		}

		template <typename LAMBDA>
		void object(LAMBDA && lambda) throws(InvalidSourceException)
		{
			if (_skipIfNot('{')) return;
			char oldchr = _open('}');
			while (m_closeCharacter != 0)
			{
				lambda(_getName());
			}
			m_closeCharacter = oldchr;
			_closeTest();
		}

		template <typename LAMBDA>
		void fields(LAMBDA && lambda) throws(InvalidSourceException)
		{
			using param_t = remove_reference_t<meta::typeAt<typename meta::function<LAMBDA>::args_t, 0>>;
			JsonFieldCaller<param_t>::call(this, lambda);
		}

	private:
		char _open(char closechr) throws(InvalidSourceException);
		kr::TText _getName() throws(InvalidSourceException);
		void _skipNumber() throws(InvalidSourceException);
		bool _skipIfNot(char chr) throws(InvalidSourceException);
		void _readNumber(TText * dest, uint * radix, bool * negative) throws(InvalidSourceException);
		void _closeTest() throws(InvalidSourceException);

		char m_closeCharacter;

		void _nextIsNull() throws(InvalidSourceException);
		void _nextIs(char chr) throws(InvalidSourceException);
		char _nextIs_y(kr::Text chr) throws(InvalidSourceException);
	};

	struct JsonFieldDone{};

	class JsonField
	{
	private:
		JsonParser * m_parser;
		Text m_name;
		
	public:
		JsonField(JsonParser * parser, Text name) noexcept
			:m_parser(parser), m_name(name)
		{
		}
		template <typename T>
		void operator ()(Text name, T * value)
		{
			if (name != m_name) return;
			m_parser->read(value);
			throw JsonFieldDone();
		}
		template <typename T>
		void operator ()(Text name, const KeepPointer<T> &value)
		{
			if (name != m_name) return;
			m_parser->read<Keep<T>>(value);
			throw JsonFieldDone();
		}
		template <typename LAMBDA>
		void operator ()(Text name, LAMBDA && lambda)
		{
			if (name != m_name) return;
			m_parser->fields(lambda);
			throw JsonFieldDone();
		}
	};

	class JsonArray
	{
	private:
		JsonParser * m_parser;
		size_t m_index;

	public:
		JsonArray(JsonParser * parser, size_t index) noexcept
			:m_parser(parser), m_index(index)
		{
		}
		size_t getIndex() noexcept
		{
			return m_index;
		}
		template <typename T>
		void operator ()(T * value)
		{
			m_parser->read(value);
			throw JsonFieldDone();
		}
		template <typename T>
		void operator ()(size_t index, T * value)
		{
			if (index != m_index) return;
			m_parser->read(value);
			throw JsonFieldDone();
		}
		template <typename LAMBDA>
		void operator ()(size_t index, LAMBDA && lambda)
		{
			if (index != m_index) return;
			m_parser->fields(lambda);
			throw JsonFieldDone();
		}
		template <typename LAMBDA>
		void operator ()(LAMBDA && lambda)
		{
			m_parser->fields(lambda);
			throw JsonFieldDone();
		}
	};

	template <typename T>
	struct JsonParsable
	{
		static void parse(JsonParser & parser, T * dest) noexcept
		{
			dest->parseJson(parser);
		}
	};

	template <typename T>
	struct JsonParsable<Keep<T>>
	{
		static void parse(JsonParser & parser, Keep<T> * dest) noexcept
		{
			T * p = _new T;
			*dest = p;
			JsonParsable<T>::parse(parser, p);
		}
	};

	template <>
	struct JsonParsable<AText>
	{
		static void parse(JsonParser & parser, AText * dest) noexcept
		{
			*dest = parser.text();
		}
	};

	// It's impossible to use, because of TText order problem
	//template <>
	//struct JsonParsable<TText>
	//{
	//	static void parse(JsonParser& parser, TText* dest) noexcept
	//	{
	//		*dest = parser.ttext();
	//	}
	//};

	template <>
	struct JsonParsable<AText16>
	{
		static void parse(JsonParser & parser, AText16 * dest) noexcept
		{
			*dest = parser.text16();
		}
	};

	template <typename T>
	struct JsonParsable<Array<T>>
	{
		static void parse(JsonParser & parser, Array<T> * dest) noexcept
		{
			parser.array([&](size_t) { dest->push(parser.read<T>()); });
		}
	};

	template <typename Value>
	struct JsonParsable<Map<Text, Value>>
	{
		static void parse(JsonParser & parser, Map<Text, Value> * dest) noexcept
		{
			parser.object([&](Text name) { dest->insert(name, parser.read<Value>()); });
		}
	};

	template <>
	struct JsonParsable<bool>
	{
		inline static void parse(JsonParser & parser, bool * dest) noexcept
		{
			*dest = parser.boolean();
		}
	};
	template <>
	struct JsonParsable<int>
	{
		inline static void parse(JsonParser & parser, int * dest) noexcept
		{
			*dest = parser.integer();
		}
	};
	template <>
	struct JsonParsable<uint>
	{
		inline static void parse(JsonParser & parser, uint * dest) noexcept
		{
			*dest = parser.uinteger();
		}
	};
	template <>
	struct JsonParsable<int64_t>
	{
		inline static void parse(JsonParser & parser, int64_t * dest) noexcept
		{
			try
			{
				*dest = parser.integer64();
			}
			catch (InvalidSourceException&)
			{
				*dest = parser.ttext().to_int64();
			}
		}
	};
	template <>
	struct JsonParsable<uint64_t>
	{
		inline static void parse(JsonParser & parser, uint64_t * dest) noexcept
		{
			try
			{
				*dest = parser.uinteger64();
			}
			catch (InvalidSourceException&)
			{
				*dest = parser.ttext().to_uint64();
			}
		}
	};
	template <>
	struct JsonParsable<float>
	{
		inline static void parse(JsonParser & parser, float * dest) noexcept
		{
			*dest = parser.floatNumber();
		}
	};
	template <>
	struct JsonParsable<double>
	{
		inline static void parse(JsonParser & parser, double * dest) noexcept
		{
			*dest = parser.floatNumber();
		}
	};

	template <typename LAMBDA>
	void JsonFieldCaller<JsonField>::call(JsonParser * parser, LAMBDA && lambda) throws(InvalidSourceException)
	{
		if (parser->_skipIfNot('{')) return;
		char oldchr = parser->_open('}');
		while (parser->m_closeCharacter != 0)
		{
			TText text = parser->_getName();
			try
			{
				{
					JsonField field(parser, text);
					lambda(field);
				}
				parser->skipValue();
			}
			catch (JsonFieldDone&)
			{
			}
		}
		parser->m_closeCharacter = oldchr;
		parser->_closeTest();
	}

	template <typename LAMBDA>
	void JsonFieldCaller<JsonArray>::call(JsonParser * parser, LAMBDA && lambda) throws(InvalidSourceException)
	{
		if (parser->_skipIfNot('[')) return;
		char oldchr = parser->_open(']');
		size_t idx = 0;
		while (parser->m_closeCharacter != 0)
		{
			try
			{
				lambda(JsonArray(parser, idx++));
				parser->skipValue();
			}
			catch (JsonFieldDone&)
			{
			}
		}
		parser->m_closeCharacter = oldchr;
		parser->_closeTest();
	}

}

