import tdd20;
using namespace TDD20;

#include <fstream>
#include <filesystem>

//#include <scope> // doesn't exist yet, so
namespace std      // roll yer own minimal scope_exit
{
	template<class EF> class scope_exit
	{
		EF func;
	public:
		explicit scope_exit(EF&& f) noexcept(std::is_nothrow_move_constructible_v<EF>) : func(std::forward<EF>(f)) {}
		~scope_exit() noexcept { try { func(); } catch (...) {} }
	};
}
// demo per test initialize and teardown by creating/deleting a tempfile 
std::filebuf MakeTempFile()
{
	std::filebuf fb;
	if (!fb.open(std::filesystem::current_path() / "tempfile.txt", std::ios::in | std::ios::out | std::ios::trunc))
		throw std::runtime_error("failed to open 'tempfile.txt'");
	return fb;
}
void DeleteTempFile(std::filebuf& fb)
{
	fb.close();
	std::filesystem::remove(std::filesystem::current_path() / "tempfile.txt");
}
Test demoingPerTestInitializationAndCleanup[] =
{
	{"YetMoreTests::Test", [] {

		auto tempfile = MakeTempFile();
		std::scope_exit cleanup{ [&tempfile]() { DeleteTempFile(tempfile); } };

		// do work, assert, etc.
		tempfile.sputn("hello\n", 6);
	}},
};


static void* p = nullptr;
Test demoingSuiteInitializationAndCleanup[] =
{
	{"SomeMoreTests::Initialize",[]{ p = ::operator new(7); }},
	{"SomeMoreTests::TestOne",   []{ }},
	{"SomeMoreTests::TestTwo",   []{ }},
	{"SomeMoreTests::TestThree", []{ }},
	{"SomeMoreTests::TestFour",  []{ }},
	{"SomeMoreTests::TestFive",  []{ }},
	{"SomeMoreTests::TestSix",   []{ }},
	{"SomeMoreTests::TestSeven", []{ }},
	{"SomeMoreTests::TestEight", []{ }},
	{"SomeMoreTests::Cleanup",   []{ ::operator delete(p); }},
};

namespace MyNamespace
{
	enum Color { Red, Green, Blue };
}
namespace TDD20
{
	template <> inline std::string ToString(const MyNamespace::Color& c) // a custom helper for your own type, enum Color, in this case
	{
		switch (c) {
		case MyNamespace::Red:   return "Red";
		case MyNamespace::Green: return "Green";
		case MyNamespace::Blue:  return "Blue";
		default:                 return "Unknown Color";
		}
	}
}
Test demoingCustomToStringForAUserDefinedType[] =
{
	{"show how to write a helper for your own type, an enum Color, in this case", [] { Assert::AreEqual(MyNamespace::Red, "Red"); }},
};

Test aTestNeedNotBePartOfArray("a test with two different types", []() { Assert::AreEqual(42, "42"); });
