import tdd20;
using namespace TDD20;


//#include <scope> // doesn't exist yet, so roll our own minimal scope_exit
template<typename F> class scope_exit
{
	F f;
public:
	explicit scope_exit(F&& f) noexcept : f(f) {}
	~scope_exit() { f(); } // if it throws while in a throw, it'll terminate, but it's better than swallowing an exception silently (for a unit testing harness)
};


// a test suite where a file is writable on entry to each test lambda and is cleaned up after exit
#include <fstream>
class FileIOTest : public Test
{
    static constexpr char TestFile[] = "TestData.txt";
    std::ofstream file;
private:
    void TestMethodInitialize() { file.open(TestFile); }
    void TestMethodCleanup()
    {
        if (file.is_open())
            file.close();
        std::filesystem::remove(TestFile);
    }
public:
    FileIOTest(const std::string& name, std::function<void(std::ofstream& file)> func)
        : Test(name, [this, func]() {
                                        TestMethodInitialize();
                                        scope_exit always([this]() { TestMethodCleanup(); });
                                        func(file);
                                    })
    {}
} fileIOTests[] = {
    {"File IO test 1: write \"Hello, world!\"",[](auto& file)
        {
            file << "Hello, world!\n";
            file.flush();
            Assert::IsTrue(file.good());
        }
    },
    {"File IO test 2: write several lines",[](auto& file)
        {
            file << "Line 1\n";
            file << "Line 2\n";
            file << "Line 3\n";
            file.flush();
            Assert::IsTrue(file.good());
        }
    },
};


// how to write your own ToString helper

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
