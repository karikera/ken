#include "stdafx.h"
#include "CppUnitTest.h"
#include <KR3/main.h>
#include <KR3/util/process.h>
#include <KR3/io/bufferedstream.h>
#include <KRUtil/text/slash.h>
#include <KRUtil/text/template.h>
#include <KRUtil/fs/path.h>
#include <KRUtil/fs/installer.h>
#include <KRUtil/fs/file.h>
#include <KR3/data/idmap.h>
#include <KRV8/easyv8.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace kr;

class Test : public JsObject<Test>
{
public:
	int m_n;

	Test(const JsArguments & args) :JsObject(args)
	{
		cout << args[0].cast<int>();
		cout << args[1].cast<int>();
	}

	~Test()
	{
		cout << "Test - destroyed" << endl;
	}

	Test* test(int a, int b, int c)
	{
		cout << a << b << c;
		return this;
	}

	static Text16 getClassName() noexcept
	{
		return u"Test";
	}
	static void initMethods(JsClass<Test> * cls) noexcept
	{
		cls->setMethod(u"test", &Test::test);
		cls->setAccessor(u"test2", &Test::m_n);
		cls->setStaticMethod(u"test3", [](int test) { cout << test; });
		cls->setStaticReadOnlyAccessor(u"ttt", [](V8Object _this)->JsAny {
			return undefined;
		});
	}
};

class Test2 :public JsObject<Test2, Test>
{
public:
	int m_n2;

	Test2(const JsArguments & args) : JsObject(args)
	{
	}

	~Test2()
	{
		cout << "Test2 - destroyed" << endl;
	}

	static Text16 getClassName() noexcept
	{
		return u"Test2";
	}
	static void initMethods(JsClass<Test2> * cls) noexcept
	{
		cls->setAccessor(u"test3", &Test2::m_n2);
	}
};

#define JAVASCRIPT(script) u#script

namespace test
{
	TEST_CLASS(V8UnitTest)
	{
	public:
		TEST_METHOD(scriptTest)
		{
			AText out;
			{
				JsContext v8, v82;
				try
				{
					V8Private a, b;

					v8.enter();
					V8Object v8o = v8.global();

					int n = Test::classObject.getFunction().internalFieldCount();
					v8o.setFunction(u"log", [&out](AText str, AText str2)->Test* {
						out << str << str2;
						return nullptr;
					});
					v8o.setFunction(u"log2", [](Test * t) {
						int a = 0;
					});
					v8o.set(b, (Text16)u"test2");
					v8.exit();

					v82.enter();
					V8Object v82o = v82.global();
					v82o.setFunctionRaw(u"log", [](const JsArguments& args)->JsAny { cout << args[0].cast<AText>() << args[1].cast<AText>(); return undefined; });
					v82o.set(u"test", v8o);
					v82.exit();

					v8.enter();
					Test * tv = Test::newInstance();
					v8o.set(u"tv", tv);
					deleteAligned(tv);
					v8.run(JAVASCRIPT(
						log(Test.ttt, "\n");
					log(tv);
					tv.test(0, 1, 2);
					log("\n", "");
					var a = 6;
					log(1.4 | 0, "2");
					var t = new Test(3, 4);
					Test.prototype.test2 = 4;
					log2();
					t.test2 = 8;
					t.test(5, 6, 7).test(8, 90);
					t.test2 = "7";
					));
					v8.run(JAVASCRIPT(
						function callLog(a, b)
					{
						log(a, b);
					}
					function callLog2(a, b)
					{
						log(a, b);
					}
					));
					v8o.callRaw(u"callLog", JsArgumentsIn({ (JsAny)2, (JsAny)3 }));
					v8o.call<void>(u"callLog2", 4, 5);
					v8.exit();

					v82.enter();
					v82.run(JAVASCRIPT(
						var b = 1;
					test.log(test.a, test.t.test2);
					var test2 = new Test2(8, 9);
					test.Test.test3(0);
					test.log("\n", typeof(test.t.test2) + "\n");
					test.t = null;
					test2 = null;
					));
					v82.exit();
				}
				catch (JsException &e)
				{
					udout << u"ERR " << e.message.c_str() << endl;
					udout.flush();
				}

			}
			JsContext::terminate();
		}
	};
}