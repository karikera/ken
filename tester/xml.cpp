#include <KR3/main.h>
#include <KR3/util/callable.h>

#ifdef WIN32

#include <KRUtil/fs/file.h>
#include <KRUtil/parser/parser.h>

namespace kr
{
	namespace meta
	{
		template <typename LAMBDA, typename ... ARGS>
		void typesGroupLoop(const LAMBDA & lambda, const ARGS & ... args)
		{
			using args_t = typename function<LAMBDA>::args_t;
			static_assert(sizeof ... (args) % args_t::size == 0, "Unmatch lambda arguments count");

			types<ARGS ...> values(args ...);
		}
	}
	class XmlParser : public Parser
	{
	public:
		template <typename LAMBDA>
		XmlParser(io::VIStream<char> is, LAMBDA warningcb);
		~XmlParser() noexcept;

		using Parser::getLine;
		void skip();

		template <typename DATACB>
		void data(const DATACB & datacb) noexcept
		{
			if (m_latestTag == nullptr)
			{
				m_warningCallback("read data from root");
				return;
			}
			skipWhiteSpace();
			datacb(_readData());
			//_closeTag();
		}
		template <typename TAGCB>
		void tag(const TAGCB & tagcb) noexcept
		{
			Text parentTag = m_latestTag;
			for (;;)
			{
				m_is.skipwith('<');
				if (m_is.nextIs('/'))
				{
					if (_testClose()) return;
					continue;
				}
				TSZ tag = _openTag();
				tagcb((Text)tag);
				m_latestTag = parentTag;
				//				_closeTag();
			}
		}
		template <typename TAGCB, typename DATACB>
		void tag(const TAGCB & tagcb, const DATACB & datacb) noexcept
		{
			Text parentTag = m_latestTag;
			for (;;)
			{
				skipWhiteSpace();
				if (m_is.nextIs('<'))
				{
					if (m_is.nextIs('/'))
					{
						if (_testClose()) return;
						continue;
					}
					TSZ tag = _openTag();
					tagcb((Text)tag);
					m_latestTag = parentTag;
					//_closeTag();
				}
				else
				{
					///					datacb(_readData());
				}
			}
		}

		// return false if it was self closing tag
		template <typename PROPCB>
		bool properties(const PROPCB & propcb) noexcept
		{
			for (;;)
			{
				switch (m_is.peek())
				{
				case '/':
					if (m_is.nextIs("/>")) return false;
					m_warningCallback("Invalid character / in tag");
					m_is.skip();
					continue;
				case '>':
					m_is.skip();
					return true;
				}
				//				TText name = _parseProperty();
				//				TText value = _parseValue();
				//				propcb((Text)name, (Text)value);
			}
		}

	private:
		void _parseHead() noexcept;

		TSZ _openTag();
		bool _testClose();
		TSZ _readData();

		Text m_latestTag;
		AText m_version;
		AText m_encoding;
		Lambda<sizeof(void*) * 2, void(Text)> m_warningCallback;

	};

	template <typename LAMBDA>
	XmlParser::XmlParser(io::VIStream<char> is, LAMBDA warningcb)
		:Parser(is), m_warningCallback(warningcb)
	{
		m_latestTag = nullptr;
		_parseHead();
	}
	XmlParser::~XmlParser() noexcept
	{
	}
	void XmlParser::skip()
	{
		m_is.skipwith(TSZ() << "</" << m_latestTag << '>');
	}

	void XmlParser::_parseHead() noexcept
	{
		skipWhiteSpace();
		if (!m_is.nextIs("<?"))
		{
			m_warningCallback("Cannot found ?xml tag");
		}
		else
		{
			m_is.nextIs("xml");
			mustWhiteSpace();
			properties([this](Text name, Text value) {
				if (name == "version") m_version = value;
				else if (name == "encoding") m_encoding = value;
				else
				{
					m_warningCallback(TText() << "Unknown ?xml tag property " << name);
				}
			});
		}
	}
	TSZ XmlParser::_openTag()
	{
		TSZ tsz = m_is.readto_y("> \t\r\n");
		m_latestTag = tsz;
		return tsz;
	}
	bool XmlParser::_testClose()
	{
		TSZ tagName = m_is.readwith('>');
		if (tagName == m_latestTag) return true;
		else
		{
			m_warningCallback(TSZ() << "Closing tag name unmatch" << m_latestTag << " != " << tagName);
			return false;
		}
	}
	TSZ XmlParser::_readData()
	{
		return m_is.readto('<');
	}
}

using namespace kr;

void xmlMain() noexcept
{
	io::BufferedIStream<io::FileStream<char>> fis(File::open(u"tester.vcxproj"));
	fis.skipto("KR.props");
	cout << fis.read(30) << endl;

	XmlParser parser(File::open(u"tester.vcxproj"), [](Text message) {
		cerr << message << endl;
	});
	parser.tag([&](kr::Text name) {
		cout << name << endl;
		if (parser.properties([&](kr::Text name, kr::Text value) {
			cout << "prop: " << name << ": " << value << endl;
		}))
		{
			parser.tag([&](Text tagName) {
				cout << "+" << tagName << endl;
				if (parser.properties([&](kr::Text name, kr::Text value) {}))
				{
				}
			}, [&] {});
		}
	});

}

#endif