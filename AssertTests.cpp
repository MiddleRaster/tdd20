import std;
import tdd20;
using namespace TDD20;

static void Catcher(auto&& func, const std::string& funcName, const std::string& message)
{
	try { func(); }
	catch (const AssertException& e)
	{
		Assert::AreEqual(message, e.what());
		return;
	}
	Assert::Fail("Expected '" + funcName + "' to throw an AssertException");
}

Test AssertTests[] =
{
	{"AssertTests: Fail", [](){ Catcher([]() { Assert::Fail("This is a failure message"); }, "Assert::Fail", "This is a failure message"); } },

	{"AssertTests: AreEqual doesn't throw when equal", []() {               Assert::AreEqual(42, 42); } },
	{"AssertTests: AreEqual does throw when unequal", []() { Catcher([]() { Assert::AreEqual(42, 43, "Values are not equal"); }, "Assert::AreEqual", "Expected <42> Actual <43> - Values are not equal"); } },

	{"AssertTests: AreNotEqual doesn't throw when unequal", []() {           Assert::AreNotEqual(42, 43); } },
	{"AssertTests: AreNotEqual does throw when equal", []() { Catcher([]() { Assert::AreNotEqual(42, 42, "Values are equal"); }, "Assert::AreNotEqual", "Unexpected equality <42> - Values are equal"); } },

	{"AssertTests: AreWithin doesn't throw when within tolerance",  []() {                Assert::AreWithin(4.2, 4.3, 0.15); } },
	{"AssertTests: AreWithin does throw when not within tolerance", []() { Catcher([]() { Assert::AreWithin(4.2, 4.3, 0.05, "Values are out of tolerance"); }, "Assert::AreWithin", "Expected <4.200000000000000> to be within <0.050000000000000> of <4.300000000000000> - Values are out of tolerance"); } },

	{"AssertTests: IsFalse doesn't throw when false", []() {            Assert::IsFalse(false); } },
	{"AssertTests: IsFalse does throw when true", []() { Catcher([]() { Assert::IsFalse(true, "Value is true"); }, "Assert::IsFalse", "Expected <false> Actual <true> - Value is true"); } },

	{"AssertTests: IsTrue doesn't throw when true", []() {              Assert::IsTrue(true); } },
	{"AssertTests: IsTrue does throw when false", []() { Catcher([]() { Assert::IsTrue(false, "Value is false"); }, "Assert::IsTrue", "Expected <true> Actual <false> - Value is false"); } },

	{"AssertTests: ExpectingException doesn't throw when correct exception is caught", []() { Assert::ExpectingException<std::exception>([]() { throw std::logic_error("boo"); }); } },
	{"AssertTests: ExpectingException does throw when no exception is thrown",    []() { Catcher([]() { Assert::ExpectingException<std::exception>([]() { /* no throw */ },    "no exception thrown"); }, "Assert::ExpectingException",            "no exception thrown; was expecting exception of type 'class std::exception' - no exception thrown"); } },
	{"AssertTests: ExpectingException does throw when wrong exception is thrown", []() { Catcher([]() { Assert::ExpectingException<std::exception>([]() {       throw 7; }, "wrong exception thrown"); }, "Assert::ExpectingException", "exception of wrong type thrown; was expecting exception of type 'class std::exception' - wrong exception thrown"); } },
};