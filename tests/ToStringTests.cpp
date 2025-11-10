import tdd20;
using namespace TDD20;

Test ToStringTests[] =
{
//	{"ToString: won't compile without user specialization", []() { struct X{} x; ToString(x); } },
	{"ToString: generic pointer return hex value", []()
		{
			int* p = nullptr;
			Assert::AreEqual("0x0", ToString(p));
		}
	},
	{"ToString: bool true", []() { Assert::AreEqual("true",  ToString(true )); } },
	{"ToString: bool false",[]() { Assert::AreEqual("false", ToString(false)); } },
	// do every integral type:
	{"ToString: max             char  ", []() { Assert::AreEqual("127",                  ToString(std::numeric_limits<              char>::max())); } },
	{"ToString: min             char  ", []() { Assert::AreEqual("-128",                 ToString(std::numeric_limits<              char>::min())); } },
	{"ToString: max    unsigned char  ", []() { Assert::AreEqual("255",                  ToString(std::numeric_limits<     unsigned char>::max())); } },
	{"ToString: min    unsigned char  ", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<     unsigned char>::min())); } },
	{"ToString: max      signed char  ", []() { Assert::AreEqual("127",                  ToString(std::numeric_limits<       signed char>::max())); } },
	{"ToString: min      signed char  ", []() { Assert::AreEqual("-128",                 ToString(std::numeric_limits<       signed char>::min())); } },
	{"ToString: max            wchar_t", []() { Assert::AreEqual("65535",                ToString(std::numeric_limits<           wchar_t>::max())); } },
	{"ToString: min            wchar_t", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<           wchar_t>::min())); } },
	{"ToString: max            char8_t", []() { Assert::AreEqual("255",                  ToString(std::numeric_limits<           char8_t>::max())); } },
	{"ToString: min            char8_t", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<           char8_t>::min())); } },
	{"ToString: max           char16_t", []() { Assert::AreEqual("65535",                ToString(std::numeric_limits<          char16_t>::max())); } },
	{"ToString: min           char16_t", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<          char16_t>::min())); } },
	{"ToString: max           char32_t", []() { Assert::AreEqual("4294967295",           ToString(std::numeric_limits<          char32_t>::max())); } },
	{"ToString: min           char32_t", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<          char32_t>::min())); } },
	{"ToString: max              short", []() { Assert::AreEqual("32767",                ToString(std::numeric_limits<             short>::max())); } },
	{"ToString: min              short", []() { Assert::AreEqual("-32768",               ToString(std::numeric_limits<             short>::min())); } },
	{"ToString: max     unsigned short", []() { Assert::AreEqual("65535",                ToString(std::numeric_limits<    unsigned short>::max())); } },
	{"ToString: min     unsigned short", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<    unsigned short>::min())); } },
	{"ToString: max                int", []() { Assert::AreEqual( "2147483647",          ToString(std::numeric_limits<               int>::max())); } },
	{"ToString: min                int", []() { Assert::AreEqual("-2147483648",          ToString(std::numeric_limits<               int>::min())); } },
	{"ToString: max       unsigned int", []() { Assert::AreEqual("4294967295",           ToString(std::numeric_limits<      unsigned int>::max())); } },
	{"ToString: min       unsigned int", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<      unsigned int>::min())); } },
	{"ToString: max               long", []() { Assert::AreEqual( "2147483647",          ToString(std::numeric_limits<              long>::max())); } },
	{"ToString: min               long", []() { Assert::AreEqual("-2147483648",          ToString(std::numeric_limits<              long>::min())); } },
	{"ToString: max      unsigned long", []() { Assert::AreEqual("4294967295",           ToString(std::numeric_limits<     unsigned long>::max())); } },
	{"ToString: min      unsigned long", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<     unsigned long>::min())); } },
	{"ToString: max          long long", []() { Assert::AreEqual( "9223372036854775807", ToString(std::numeric_limits<         long long>::max())); } },
	{"ToString: min          long long", []() { Assert::AreEqual("-9223372036854775808", ToString(std::numeric_limits<         long long>::min())); } },
	{"ToString: max unsigned long long", []() { Assert::AreEqual("18446744073709551615", ToString(std::numeric_limits<unsigned long long>::max())); } },
	{"ToString: min unsigned long long", []() { Assert::AreEqual("0",                    ToString(std::numeric_limits<unsigned long long>::min())); } },
	// do every floating_point type:
	{"ToString: min       float", []() { Assert::AreEqual("0.000000000000000", ToString(std::numeric_limits<      float>::min())); } },
	{"ToString: min      double", []() { Assert::AreEqual("0.000000000000000", ToString(std::numeric_limits<     double>::min())); } },
	{"ToString: min long double", []() { Assert::AreEqual("0.000000000000000", ToString(std::numeric_limits<long double>::min())); } },
	{"ToString: max       float", []() { Assert::AreEqual("340282346638528859811704183484516925440.000000000000000", ToString(std::numeric_limits<float>::max())); } },
	{"ToString: max      double", []() { Assert::AreEqual("179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000000000000", ToString(std::numeric_limits<     double>::max())); } },
	{"ToString: max long double", []() { Assert::AreEqual("179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000000000000", ToString(std::numeric_limits<long double>::max())); } },
	// do every string type:
	{"ToString:  std::string", []() { Assert::AreEqual("Hi, there", ToString( std::string ("Hi, there"))); }},
	{"ToString:       char *", []() { Assert::AreEqual("Hi, there", ToString(              "Hi, there" )); }},
	{"ToString: std::wstring", []() { Assert::AreEqual("Hi, there", ToString(std::wstring(L"Hi, there"))); }},
	{"ToString:    wchar_t *", []() { Assert::AreEqual("Hi, there", ToString(             L"Hi, there" )); }},
};
