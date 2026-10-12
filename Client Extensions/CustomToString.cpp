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
        default:                 return std::format("Unknown MyNamespace::Color ({})", static_cast<int>(c));
        }
    }
}
Test demoingCustomToStringForAUserDefinedType[] =
{
    {"how to write a helper for an enum Color", [] { Assert::AreEqual   (MyNamespace::Red, "Red");  }},
    {"enum mismatch",                           [] { Assert::AreNotEqual(MyNamespace::Red, "Blue");  }},
    {"enum mismatch 2",                         [] { Assert::AreNotEqual(MyNamespace::Red, MyNamespace::Green); }},
    {"unknown enum value",                      [] { Assert::AreEqual  ((MyNamespace::Color)7, "Unknown MyNamespace::Color (7)"); }},
};
