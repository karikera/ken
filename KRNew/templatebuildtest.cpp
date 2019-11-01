#include <KR3/main.h>
#include <KR3/meta/text.h>
#include <KR3/data/linkedlist.h>
#include <KR3/util/path.h>
#include <KR3/util/wide.h>
#include <KR3/text/slash.h>
#include <KR3/parser/parser.h>
#include <KR3/fs/file.h>

using namespace kr;

inline void templatebuildtest() noexcept
{
	using Utf16ToUtf8 = Utf16ToMultiByte<Charset::Utf8>;
	using Utf8ToUtf16 = MultiByteToUtf16<Charset::Utf8>;
	using Utf32ToUtf8 = Utf32ToMultiByte<Charset::Utf8>;
	using Utf8ToUtf32 = MultiByteToUtf32<Charset::Utf8>;
	using Utf16ToAnsi = Utf16ToMultiByte<Charset::Default>;
	using AnsiToUtf16 = MultiByteToUtf16<Charset::Default>;
	using Utf32ToAnsi = Utf32ToMultiByte<Charset::Default>;
	using AnsiToUtf32 = MultiByteToUtf32<Charset::Default>;
	LinkedList<Node<int>> test;
	test.create(10);
	for (Node<int>& t : test.reverse())
	{
	}
	Array<int> test2;
	test2.resize(10);
	for (int& t : test2.reverse())
	{
	}
	Chain<Node<int>> test3;
	test3.create(10);
	for (Node<int>& t : test3.reverse())
	{
	}

	Charset cs = Charset::Default;
	TText16 utf16;
	TText _value = {'1', '\0'};
	CHARSET_CONSTLIZE(cs, {
		utf16 << (MultiByteToUtf16<cs>)_value;
	});

	Text stream = _value;
	stream.readwith_e('*');
	stream.readwith_L(path.isSeperator);

	static const auto filter = meta::literal_as<char>("\"\'\n\r,");
	static const auto from = meta::literal_as<char>("\"");
	static const auto to = meta::literal_as<char>("\"\"");

	View<char> a = (View<char>)from;

	TSZ dest;
	dest << currentDirectory;
	
	TSZ16 dest16;
	wide(dest16);

	Parser parser(&a);
	parser.readToSpace();

	Text ori_text = "\0\"\"\"\"sf\\ghsfgh\'\'\rsfghsgh\r\r\nsfhsfgh\n\r\rsfg\t\t\thsfgh\n\nsghsgh\r\rsghsfgh\n";
	AText mid_text = addSlashes(ori_text);

	ori_text.splitIterable('0');
	ori_text.splitIterable("0");
	ori_text.reverseSplitIterable('0');
	ori_text.reverseSplitIterable("0");

	File* file;
	mid_text << file->stream<char>()->read(100);
}
