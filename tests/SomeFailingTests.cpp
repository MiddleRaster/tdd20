import tdd20;
using namespace TDD20;

Test someFailingTests[] = 
{
	{"generic pointer test",                                                                  []{ struct X {} a,b; Assert::AreEqual(&a, &b); } }, // will compare hex addresses
	{"no file/line info when catching an exception that doesn't inherit from std::exception", []{ throw 7; } },
	{"no file/line info when catching an exception that does    inherit from std::exception", []{ throw std::logic_error ("logic error exception"); } },
	{"just failing right off",                                                                []{ Assert::Fail       ("Yep, failed"); } },
	{"an Assert::AreNotEqual test",                                                           []{ Assert::AreNotEqual("hello", "hello"); }},
	{"an IsWithin test using 15 decimal places",                                              []{ Assert::AreWithin  (std::numbers::pi, 3.14, 0.0001, "not close enough"); } },
	{"an IsFalse test",                                                                       []{ Assert::IsFalse    (true,  "this should be false"); } },
	{"an IsTrue test",                                                                        []{ Assert::IsTrue     (false, "this should be true" ); } },
};
