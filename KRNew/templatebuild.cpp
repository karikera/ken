#include "stdafx.h"
#include <KR3/main.h>
#include <KR3/meta/text.h>
#include <KR3/data/linkedlist.h>
#include <KR3/util/path.h>
#include <KR3/util/wide.h>
#include <KR3/text/slash.h>
#include <KR3/parser/parser.h>
#include <KR3/fs/file.h>
#include <KR3/io/selfbufferedstream.h>
#include <KR3/util/bufferqueue.h>
#include <KR3/data/crypt.h>
#include <KR3/wl/windows.h>
#include <KR3/util/process.h>

using namespace kr;

inline void buildTest() noexcept
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
	dest << a.filter([](char chr) { return chr; });
	
	TSZ16 dest16;
	wide(dest16);

	Parser parser(&a);
	{
		TSZ zzz = parser.readToSpace();
		// 확장자가 없으면 디렉토리를 찾는다?
	}

	Text ori_text = "\0\"\"\"\"sf\\ghsfgh\'\'\rsfghsgh\r\r\nsfhsfgh\n\r\rsfg\t\t\thsfgh\n\nsghsgh\r\rsghsfgh\n";

	ori_text.splitIterable('0');
	ori_text.splitIterable("0");
	ori_text.reverseSplitIterable('0');
	ori_text.reverseSplitIterable("0");

	File* file = File::open(u"test");
	AText mid_text;
	mid_text << file->stream<char>()->read(100);

	Must<io::FIStream<char, false>> fiskeep = _new io::FIStream<char, false>(file);
	io::FIStream<char, false> &fis = *fiskeep;
	fis.request(100);

	pcstr16 src = nullptr;
	int line = 0;
	udout << src;
	udout << u'(';
	udout << line;
	udout << u")\r\n";

	a.replace<Utf8ToUtf16>(&dest16, "asd", u"asd");

	TSZ16 moduleName;
	moduleName << ModuleName<char16>() << nullterm;


	AText b = (AText)shell(nullptr, nullptr);
}

template class kr::ary::WrapImpl<kr::ary::AccessableData<char, kr::Empty>, char>;
template class kr::ary::WrapImpl<kr::ary::ReadableData<char, kr::Empty>, char>;
template class kr::ary::WrapImpl<kr::ary::AllocatedData<char, kr::Empty>, char>;
template class kr::ary::WrapImpl<kr::ary::TemporaryData<char, kr::Empty>, char>;
template class kr::ary::WrapImpl<kr::ary::WritableData<char, kr::Empty>, char>;

template class kr::ary::WrapImpl<kr::ary::AccessableData<char16, kr::Empty>, char16>;
template class kr::ary::WrapImpl<kr::ary::ReadableData<char16, kr::Empty>, char16>;
template class kr::ary::WrapImpl<kr::ary::AllocatedData<char16, kr::Empty>, char16>;
template class kr::ary::WrapImpl<kr::ary::TemporaryData<char16, kr::Empty>, char16>;
template class kr::ary::WrapImpl<kr::ary::WritableData<char16, kr::Empty>, char16>;

template class kr::AddSlashes<char>;
template class kr::StripSlashes<char>;
template class kr::AddSlashes<kr::char16>;
template class kr::StripSlashes<kr::char16>;

template class kr::SplitIterator<void>;
template class kr::TextSplitIterator<void>;
template class kr::ReverseSplitIterator<void>;
template class kr::ReverseTextSplitIterator<void>;
template class kr::LoopIterator<void>;

template class kr::SplitIterator<char>;
template class kr::TextSplitIterator<char>;
template class kr::ReverseSplitIterator<char>;
template class kr::ReverseTextSplitIterator<char>;
template class kr::LoopIterator<char>;

template class kr::SplitIterator<char16>;
template class kr::TextSplitIterator<char16>;
template class kr::ReverseSplitIterator<char16>;
template class kr::ReverseTextSplitIterator<char16>;
template class kr::LoopIterator<char16>;

template class kr::ToConvert<Charset::Utf8, char16>;
template class kr::ToConvert<Charset::Default, char16>;

template class kr::encoder::Encoder<kr::encoder::Uri, char, char>;
template class kr::encoder::Encoder<kr::encoder::HtmlEntity, char, char>;
template class kr::encoder::Encoder<kr::encoder::Hex, char, void>;
template class kr::encoder::Encoder<kr::encoder::Base64, char, void>;

template class kr::encoder::Hasher<kr::encoder::Sha1Context>;
template class kr::encoder::Hasher<kr::encoder::Sha256Context>;
template class kr::encoder::Hasher<kr::encoder::Md5Context>;

template struct kr::Names<char>;
template struct kr::Names<kr::char16>;

template class kr::io::SizeOStream<void>;
template class kr::io::SizeOStream<char>;
template class kr::io::SizeOStream<kr::char16>;
