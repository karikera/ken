#pragma once

#include <KR3/main.h>
NEED_FILESYSTEM

#include <KR3/data/map.h>
#include <KR3/io/bufferedstream.h>
#include "file.h"
#include "../parser/jsonparser.h"

namespace kr
{

	class JsonConfig
	{
	private:
		class ValLink;
		template <typename T, typename GET, typename SET>
		class ValLinkLambda;

		Array<ValLink*> m_linkList;
		ReferenceMap<AText, ValLink*> m_nameMap;
	public:

		JsonConfig() noexcept;
		~JsonConfig() noexcept;
		template <typename GET, typename SET>
		void link(Text name, GET get, SET set) noexcept;
		template <typename T>
		void link(Text name, T * valueptr) noexcept;
		template <typename T>
		void linkSz(Text name, T * valueptr) noexcept;
		template <size_t sz>
		void linkSzBuf(Text name, char (&buf)[sz]) noexcept;
		template <size_t sz>
		void linkSzBuf(Text name, char16(&buf)[sz]) noexcept;
		void save(pcstr16 filename) throws(Error);
		void load(pcstr16 filename) throws(EofException, InvalidSourceException, Error);
	};


	class JsonConfig::ValLink
	{
	public:
		ValLink(Text _name) noexcept;
		virtual ~ValLink() noexcept;
		virtual void save(io::FOStream<char> * fout) = 0;
		virtual void load(JsonParser * parser) = 0;

		AText name;
		size_t index;
	};
	template <typename T, typename GET, typename SET>
	class JsonConfig::ValLinkLambda :public JsonConfig::ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": " << m_get();
		}
		void load(JsonParser * parser) override
		{
			m_set((T)parser->integer());
		}
	};

	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<qword, GET, SET>:public JsonConfig::ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": " << m_get();
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->uinteger64());
		}
	};
	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<float, GET, SET>
		:public JsonConfig::ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": " << m_get();
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->floatNumber());
		}
	};
	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<bool, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": " << m_get();
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->boolean());
		}
	};

	template <typename T, typename GET, typename SET>
	class JsonConfig::ValLinkLambda<Array<T>, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": [";
			Array<T> arr = m_get();
			View<T> ref = arr;
			if (arr.empty())
			{
				*fout << ']';
				return;
			}
			*fout << *ref++;
			for (const T & v : ref)
			{
				*fout << ", " << v;
			}
			*fout << ']';
		}
		void load(JsonParser * parser) override
		{
			Array<T> values;
			parser->array([&](size_t) {
				values.push(parser->integer()); // XXX: 임시로 숫자만 하게 했다.
			});
			m_set(move(values));
		}
	};
	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<AText16, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": \"";
			m_get().replace<Utf16ToUtf8>(fout, u"\"", "\\\"");
			*fout << "\"";
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->text16());
		}
	};
	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<AText, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": \"";
			m_get().replace(fout, "\"", "\\\"");
			*fout << "\"";
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->text());
		}
	};

	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<Text16, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": \"";
			m_get().replace<WideToUtf8>(fout, u"\"", "\\\"");
			*fout << "\"";
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->text16());
		}
	};
	template <typename GET, typename SET>
	class JsonConfig::ValLinkLambda<Text, GET, SET> :public ValLink
	{
	private:
		GET m_get;
		SET m_set;

	public:

		ValLinkLambda(Text name, GET get, SET set) noexcept
			: ValLink(name), m_get(move(get)), m_set(move(set))
		{
		}
		~ValLinkLambda() noexcept override
		{
		}
		void save(io::FOStream<char> * fout) override
		{
			*fout << "\"" << name << "\": \"";
			m_get().replace(fout, "\"", "\\\"");
			*fout << "\"";
		}
		void load(JsonParser * parser) override
		{
			m_set(parser->ttext());
		}
	};

	template <typename GET, typename SET>
	void JsonConfig::link(Text name, GET get, SET set) noexcept
	{
		ValLink * link = _new ValLinkLambda<decltype(get()), GET, SET>(name, move(get), move(set));
		auto res = m_nameMap.insert(link->name, link);
		if (!res.second)
		{
			ValLink * old = res.first->second;
			m_linkList[old->index] = link;
			delete old;
			res.first->second = link;
		}
		else
		{
			link->index = m_linkList.size();
			m_linkList.push(link);
		}
	}
	template <typename T>
	void JsonConfig::link(Text name, T * valueptr) noexcept
	{
		link(name,
			[valueptr]()->T { return *valueptr; },
			[valueptr](T value) { *valueptr = value; }
		);
	}
	template <typename T>
	void JsonConfig::linkSz(Text name, T * valueptr) noexcept
	{
		link(name,
			[valueptr] { return *valueptr; },
			[valueptr](T value) {
			*valueptr = move(value);
			*valueptr << nullterm;
		});
	}
	template <size_t sz>
	void JsonConfig::linkSzBuf(Text name, char(&buf)[sz]) noexcept
	{
		link(name,
			[&buf]()->Text{ return (Text)(const char *)buf; },
			[&buf](Text text) { 
			try
			{
				ArrayWriter<char>(buf) << text << nullterm;
			}
			catch (NotEnoughSpaceException&)
			{
				buf[0] = '\0';
			}
		});
	}
	template <size_t sz>
	void JsonConfig::linkSzBuf(Text name, char16(&buf)[sz]) noexcept
	{
		link(name,
			[&buf]()->Text16 { return (Text16)(const char *)buf; },
			[&buf](Text16 text) {
			try
			{
				ArrayWriter<char16>(buf) << text << nullterm;
			}
			catch (NotEnoughSpaceException&)
			{
				buf[0] = '\0';
			}
		});
	}
}
