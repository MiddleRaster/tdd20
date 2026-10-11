import tdd20;
import std;
using namespace TDD20;

Test aTestNeedNotBePartOfArray("a test with two different types", []() { Assert::AreEqual(42, "42"); });
