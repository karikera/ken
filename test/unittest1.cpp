#include "stdafx.h"
#include "CppUnitTest.h"
#include <KR3/main.h>
#include <KR3/util/process.h>
#include <KR3/io/bufferedstream.h>
#include <KR3/text/slash.h>
#include <KR3/text/template.h>
#include <KR3/util/path.h>
#include <KR3/fs/installer.h>
#include <KR3/fs/file.h>
#include <KR3/data/idmap.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace kr;
using namespace meta;

namespace test
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		TEST_METHOD(textEquals)
		{
			using memw = memt<sizeof(wchar_t)>;
			Assert::IsTrue(memw::compare(L"abcdefg", L"abcdefg", 7) == 0);

			Assert::IsTrue(memw::compare(L"abcdefgabcdefg", L"abcdefgabcdefg", 14) == 0);
			Assert::IsTrue(memw::compare(L"abcdefgabcdefg", L"0abcdefgabcdefg" + 1, 14) == 0);
			Assert::IsTrue(memw::compare(L"0abcdefgabcdefg" + 1, L"0abcdefgabcdefg" + 1, 14) == 0);

			Assert::IsTrue(memw::compare(L"abcdefgabcdefg", L"abcdefgabcdefk", 14) < 0);
			Assert::IsTrue(memw::compare(L"abcdefgabcdefg", L"0abcdefgabcdefk" + 1, 14) < 0);
			Assert::IsTrue(memw::compare(L"0abcdefgabcdefg" + 1, L"0abcdefgabcdefk" + 1, 14) < 0);
		}

		TEST_METHOD(installer)
		{
			File::removeFullDirectory(u"../../../test/copytest_to");
			Installer installer(u"../../../test/copytest_to", u"../../../test/copytest_from");
			installer.all();
			Assert::AreEqual("aa", ((TText)File::openAsArray<char>(u"../../../test/copytest_to/a.txt")).c_str());
			Assert::AreEqual("bb", ((TText)File::openAsArray<char>(u"../../../test/copytest_to/b.txt")).c_str());
			Assert::AreEqual("dd", ((TText)File::openAsArray<char>(u"../../../test/copytest_to/c/d.txt")).c_str());
			Assert::AreEqual("aa", ((TText)File::openAsArray<char>(u"../../../test/copytest_to/c/a/a.txt")).c_str());
			Assert::AreEqual("bb", ((TText)File::openAsArray<char>(u"../../../test/copytest_to/c/b/b.txt")).c_str());
		}

		TEST_METHOD(floatToString)
		{
			Assert::AreEqual("1.2234e14", TSZ() << 122341000000000.f);
			Assert::AreEqual("-1.2234e14", TSZ() << -122341000000000.f);
			Assert::AreEqual("1.2e-8", TSZ() << 0.000000012f);
			Assert::AreEqual("-1.2e-8", TSZ() << -0.000000012f);
		}

		TEST_METHOD(tempAllocator)
		{
			Temp<byte> a;
			Temp<byte> s;
			Temp<int> b;
			Assert::AreEqual(((size_t)((int*)b) & 3), (size_t)0);
		}

		TEST_METHOD(testArray)
		{
			byte list[] = {11,4,13,8,1,2,15,5,6,7,3,10,9,14,12,16,0};
			SortedArray<byte> test;
			for (byte v : list)
			{
				test.insert(v);
			}
			for (size_t i = 0; i < sizeof(list); i++)
			{
				Assert::AreEqual((byte)i, (byte)test[i]);
			}
		}

		TEST_METHOD(shellDir)
		{
			{
				AText dest;
				Process process(Process::Shell, u"echo for test");
				io::BufferedIStream<Process> is(&process);
				dest.passThrough(&is);
				Assert::AreEqual(dest.c_str(), "for test\r\n");
			}

			{
				Process process(Process::Shell, u"dir");
				io::BufferedIStream<Process> is(&process);
				cout.passThrough(&is);
			}
		}

		TEST_METHOD(testPath)
		{
			Assert::AreEqual("basename.ext", TSZ() << path.basename("dir/name/basename.ext"));
			Assert::AreEqual("dir/name/", TSZ() << path.dirname("dir/name/basename.ext"));
			Assert::AreEqual(true, path.endsWithSeperator("dirname/"));
			Assert::AreEqual(true, path.endsWithSeperator("dirname\\"));
			Assert::AreEqual(false, path.endsWithSeperator("dirname"));
			Assert::AreEqual(true, path.startsWithSeperator("/dirname"));
			Assert::AreEqual(true, path.startsWithSeperator("\\dirname"));
			Assert::AreEqual(false, path.startsWithSeperator("dirname"));
			Assert::AreEqual(".ext.name", TSZ() << path.extname("dirname/basename.ext.name"));
			Assert::AreEqual("", TSZ() << path.extname("dirname/basename"));
			Assert::AreEqual(true, path.isSeperator('/'));
			Assert::AreEqual(true, path.isSeperator('\\'));
			Assert::AreEqual(false, path.isSeperator('?'));
			Assert::AreEqual("dir\\name\\basename.ext", path.join("dir/", "name/", "basename.ext"));
			Assert::AreEqual("\\dir\\name\\nextdir\\", path.join("/dir/", "name/", "nextdir/"));
			Assert::AreEqual("\\nextdir\\", path.join("/dir/", "../", "nextdir/"));
			Assert::AreEqual("\\..\\nextdir\\", path.join("/dir/", "../", "../nextdir/"));
			Assert::AreEqual("..\\nextdir\\", path.join("dir/", "../", "../nextdir/"));
			TSZ resolved = path.resolve("dirname/../../basename.ext");
			Assert::IsTrue(resolved.get(1) == ':');
			Assert::IsTrue(resolved.endsWith("\\basename.ext"));
			resolved.copy(path.resolve("."));
			Assert::IsTrue(resolved.get(1) == ':');
			Assert::IsTrue(!resolved.endsWith("\\"));
		}
		
		TEST_METHOD(testMin)
		{
			constexpr int v = mint(7, 1, 2, 3, 4);
			Assert::AreEqual(v, 1);
		}

		TEST_METHOD(testNewText)
		{
			AText * text = _new AText;
			delete text;
			memcheck();
		}

		TEST_METHOD(testTemplateWriter)
		{
			AText out;
			TemplateWriter<AText> writer(&out, "{{", "}}");
			writer.put("entry", "asdfasdf");
			static Text trueFalse[2] = { "false", "true" };
			writer.put("toolbar", trueFalse[true]);
			writer.write("A{{entry}}B{{toolbar}}C");

			Assert::AreEqual("AasdfasdfBtrueC", out.c_str());
		}

		TEST_METHOD(testTypesSubarray)
		{
			types<int, char, int, int> values = { 1, 'c', 3, 5 };
			auto t1 = values.subarr<1>();
			static_assert(t1.size == 3, "meta::types::subarr");
			Assert::AreEqual('c', t1.get<0>());
			Assert::AreEqual(3, t1.get<1>());
			Assert::AreEqual(5, t1.get<2>());

			auto t2 = t1.cut_copy<2>();
			static_assert(t2.size == 2, "meta::types::subarr");
			Assert::AreEqual('c', t2.get<0>());
			Assert::AreEqual(3, t2.get<1>());

			auto subbed = values.subarr_copy<1, 2>();
			static_assert(subbed.size == 2, "meta::types::subarr_copy");
			Assert::AreEqual('c', subbed.get<0>());
			Assert::AreEqual(3, subbed.get<1>());
		}

		TEST_METHOD(testSlashes)
		{
			Text ori_text = "\0\"\"\"\"sf\\ghsfgh\'\'\rsfghsgh\r\r\nsfhsfgh\n\r\rsfg\t\t\thsfgh\n\nsghsgh\r\rsghsfgh\n";
			AText mid_text = addSlashes(ori_text);
			Assert::AreEqual("\\0\\\"\\\"\\\"\\\"sf\\\\ghsfgh\\\'\\\'\\rsfghsgh\\r\\r\\nsfhsfgh\\n\\r\\rsfg\\t\\t\\thsfgh\\n\\nsghsgh\\r\\rsghsfgh\\n", mid_text.c_str());
			AText res_text = stripSlashes(mid_text);
			Assert::AreEqual(ori_text.data(), res_text.c_str());
			AText cus_text = addSlashesCustom("\"\'\\\t\t", [](char chr) {
				switch (chr)
				{
				case '\r': return 'r';
				case '\n': return 'n';
				case '\t': return 't';
				case '\0': return '0';
				case '\"': return '\"';
				case '\\': return '\\';
				default: return '\0';
				}
			});
			Assert::AreEqual("\\\"\'\\\\\\t\\t", cus_text.c_str());
		}

		TEST_METHOD(testReversePointer)
		{
			Array<int> a = { 1,2,3,4,5 };

			int n = 5;
			for (int& v : a.reverse())
			{
				Assert::AreEqual(v, n--);
			}
		}
	};
}