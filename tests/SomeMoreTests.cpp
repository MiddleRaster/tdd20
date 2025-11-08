import tdd20;
using namespace TDD20;

static int x = 42;
TDD20::Test someMoreTests[] =
{
	{"SomeMoreTests::Initialize",[]{ x = 7;     Assert::AreEqual(7, x); }},
	{"SomeMoreTests::TestOne",   []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestTwo",   []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestThree", []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestFour",  []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestFive",  []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestSix",   []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestSeven", []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::TestEight", []{ Assert::Fail("oh, I'm a failure"); }},
	{"SomeMoreTests::Cleanup",   []{ x = 42;   Assert::AreEqual(42, x); }},
};


namespace MyNamespace {
	enum Color { Red, Green, Blue };
}
namespace Detail
{
	template <> inline std::string ToString(const MyNamespace::Color& c)
	{
		switch (c) {
		case MyNamespace::Red:   return "Red";
		case MyNamespace::Green: return "Green";
		case MyNamespace::Blue:  return "Blue";
		default:                 return "Unknown Color";
		}
	}
}
TDD20::Test yetSomeMore[] =
{
	{"show how to write a helper for your own type, an enum Color, in this case", []{ Assert::AreEqual(MyNamespace::Red, MyNamespace::Green); } },
	{"a test with two different types", []{ Assert::AreEqual(42, "42"); } },
	{"a test for an expected exception", 
		[](){
			Assert::ExpectingException<std::exception>([] { throw 7; }, "and here's my custom message");
		}
	},
	
};
