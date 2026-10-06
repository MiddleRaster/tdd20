import tdd20;
import std;
using namespace TDD20;


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
