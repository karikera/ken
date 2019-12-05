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
#include <KR3/js/js.h>
#include <KR3/util/wide.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace kr;

static AText out;

class Test : public JsObjectT<Test>
{
public:
	static constexpr char16 className[] = u"Test";
	int m_n;

	Test(const JsArguments & args) :JsObjectT(args)
	{
		out << args.at<int>(0);
		out << args.at<int>(1);
	}

	~Test()
	{
		out << "Test - destroyed";
	}

	Test* test(int a, int b, int c)
	{
		out << a << b << c;
		return this;
	}

	static void initMethods(JsClassT<Test> * cls) noexcept
	{
		cls->setMethod(u"test", &Test::test);
		cls->setAccessor(u"test2", &Test::m_n);
		cls->setStaticMethod(u"test3", [](int test) { out << test; });
		cls->setStaticGetterL(u"ttt", [](JsValue _this)->JsValue {
			return 1;
		});
	}
};

class Test2 :public JsObjectT<Test2, Test>
{
public:
	static constexpr char16 className[] = u"Test2";
	int m_n2;

	Test2(const JsArguments & args) : JsObjectT(args)
	{
	}

	~Test2()
	{
		out << "Test2 - destroyed\n";
	}

	static void initMethods(JsClassT<Test2> * cls) noexcept
	{
		cls->setAccessor(u"test3", &Test2::m_n2);
	}
};

#define OUTCHECK(out_expected) Assert::IsTrue(out == out_expected, \
wide(TSZ16() << u"\nactual: " << (Utf8ToUtf16)out << u"\nexpected: " << (Utf8ToUtf16)out_expected)\
); out=nullptr;
#define JAVASCRIPT(script, res, out_expected) try{ \
JsValue actual = JsRuntime::run(u#script); \
JsValue expected = JsRuntime::run(u#res); \
Assert::IsTrue(actual == expected, wide(TSZ16() << u"(" u#script u")\nactual: " << actual.cast<AText16>() << u"\nexpected: " << expected.cast<AText16>())); \
OUTCHECK(out_expected); \
} catch (JsException & e) { Assert::Fail(wide(TSZ16() << e.toString())); } \

namespace test
{
	TEST_CLASS(JsUnitTest)
	{
	public:
		TEST_METHOD(scriptTest)
		{
			delete (_pri_::JsClassInfo*)1;

			JsRuntime runtime;
			JsContext ctx1, ctx2;
			try
			{
				// V8Private a, b;
				JsValue ctx1g;

				{
					JsContext::Scope _scope = ctx1;
					ctx1g = JsRuntime::global();
					ctx1g.set(u"testval", 0);
					ctx1g.setMethod(u"log", [](AText str, AText str2)->JsValue {
						out << str << str2;
						return nullptr;
						});
					try
					{
						JsRuntime::run(u"Promise.resolve('test').then(()=>{})");
						JsRuntime::run(u"(async()=>{ await Promise.resolve('test').then(()=>{}); log('test')  })();");
						OUTCHECK("test"); // optional Text will be null
					}
					catch (JsException& e)
					{
						Assert::Fail(wide(TSZ16() << e.toString()));
					}
					ctx1g.setMethod(u"log2", [](Test* t) {
						int a = 0;
						});
					JAVASCRIPT(log('test'), null, "test");
					// ctx1g.set(b, (Text16)u"test2");
				}
				{
					JsContext::Scope _scope = ctx2;
					JsValue ctx2g = JsRuntime::global();
					ctx2g.setMethodRaw(u"log", [](const JsArguments& args)->JsValue {
						out << args[0].cast<AText>() << args[1].cast<AText>();
						return undefined;
						});
					ctx2g.set(u"ctx1g", ctx1g);
				}
				{
					JsContext::Scope _scope = ctx1;
					Test* tv = Test::newInstance();
					OUTCHECK("00");
					ctx1g.set(u"tv", tv);
					deleteAligned(tv);
					OUTCHECK("Test - destroyed");
					JAVASCRIPT(log(Test.ttt, 'x'), null, "1x");
					JAVASCRIPT(log(tv), null, "[object Object]");
					try
					{
						JsRuntime::run(u"tv.test(0,1,2)");
						Assert::Fail(L"Access deleted class method");
					}
					catch (JsException& e)
					{
						Text16 actual = e.toString();
						Text16 expected = u"Error: this is not Test";
						Assert::IsTrue(actual == expected, wide(TSZ16() <<
							u"Message not matched.\nactual: " << actual <<
							u"\nexpected:" << expected));
					}
					JAVASCRIPT(log('x', ''), null, "x");
					JAVASCRIPT(var a = 6; a, 6, "");
					JAVASCRIPT(log(1.4 | 0, '2'), null, "12");
					JAVASCRIPT(var t = new Test(3, 4); t , t, "34");
					JAVASCRIPT(Test.prototype.test2 = 4, 4, ""); // prototype setter access
					JAVASCRIPT(log2(), undefined, "");
					JAVASCRIPT(t.test2 = 8, 8, "");
					JAVASCRIPT(t.test(5, 6, 7).test(8, 90), t, "5678900");
					JAVASCRIPT(t.test2 = '7', '7', "");
					JAVASCRIPT(function callLog(a, b) { log(a, b); }; callLog, callLog, "");
					JAVASCRIPT(function callLog2(a, b) { log(a, b); }; callLog2, callLog2, "");
					try
					{
						ctx1g.get(u"callLog").callRaw(undefined, JsArgumentsIn({ (JsValue)2, (JsValue)3 }));
					}
					catch (JsException & e)
					{
						Assert::Fail(wide(TSZ16() << e.toString()));
					}
					OUTCHECK("23");
					try
					{
						ctx1g.get(u"callLog2").call<void>(4, 5);
					}
					catch (JsException & e)
					{
						Assert::Fail(wide(TSZ16() << e.toString()));
					}
					OUTCHECK("45");
				}

				// cross context
				//{
				//	JsContext::Scope _scope = ctx2;
				//	JAVASCRIPT(var b = 1; b , 1, "");
				//	JAVASCRIPT(ctx1g.log(ctx1g.a, ctx1g.t.test2), null, "6undefined");
				//	JAVASCRIPT(var test2 = new Test2(8, 9), test2, "");
				//	JAVASCRIPT(ctx1g.Test.test3(0), undefined, "");
				//	JAVASCRIPT(ctx1g.log('\n', typeof(ctx1g.t.test2) + '\n'), 1, "");
				//	JAVASCRIPT(ctx1g.t = null, 1, "");
				//	JAVASCRIPT(test2 = null, 1, "");
				//}
			}
			catch (JsException &)
			{
				Assert::Fail(L"Unknown Javascript Exception");
			}
		}
	};
}
