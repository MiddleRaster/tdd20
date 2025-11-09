import tdd20;
using namespace TDD20;

Test someTests[] = 
{
	{"test two ints", []() { Assert::AreEqual(2, 1); } },
	{"void* test", 
		[]() {
			char a=0, b=1;
			void* p = &a;
			void* q = &b;
			Assert::AreEqual(p, q);
		}
	},
	{"throwing a raw std::exception",                                  []{ throw std::exception("can't do enums"); } },
	{"throwing an exception that doesn't inherit from std::exception", []{ throw 7; } },
	{"just failing right off",                                         []{ Assert::Fail       ("oh, I'm a failure"); } },
	{"an Assert::AreNotEqual test",                                    []{ Assert::AreNotEqual("hello", "hello", "hello == hello?"); }},
	{"an IsWithin test",                                               []{ Assert::AreWithin  (3.14159, 3.14, 0.0001, "not close enough"); } },
	{"an IsFalse test",                                                []{ Assert::IsFalse    (true,  "this should be false"); } },
	{"an IsTrue test",                                                 []{ Assert::IsTrue     (false, "this should be true" ); } },
	{"trying out TDD20::ToString",                                     []{ Assert::AreEqual   (ToString(false), ToString(true)); } },
	{"trying out wide string conversion",                              []{ Assert::AreEqual   (L"hello", "hello"); } },
};
