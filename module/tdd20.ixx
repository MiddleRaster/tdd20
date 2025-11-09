// Copyright © 2025 MiddleRaster
// Licensed under the MIT License
// See LICENSE file in the project root for full license information.

export module tdd20;

import std;

namespace Detail
{
	export template<typename T> inline std::string ToString(const T&) { static_assert(false, "test writer must write a specialization for this type"); }
	export template<typename T> inline std::string ToString(const T*) { static_assert(false, "test writer must write a specialization for this type"); }
	export template<typename T> inline std::string ToString(      T*) { static_assert(false, "test writer must write a specialization for this type"); }

	template <> inline std::string ToString(const bool& t)              { return std::to_string(t); }
	template <> inline std::string ToString(const int& t)               { return std::to_string(t); }
	template <> inline std::string ToString(const long& t)              { return std::to_string(t); }
	template <> inline std::string ToString(const long long& t)         { return std::to_string(t); }
	template <> inline std::string ToString(const short& t)             { return std::to_string(t); }
	template <> inline std::string ToString(const char& t)              { return std::to_string(t); }
	template <> inline std::string ToString(const signed char& t)       { return std::to_string(t); }
	template <> inline std::string ToString(const unsigned int& t)      { return std::to_string(t); }
	template <> inline std::string ToString(const unsigned long& t)     { return std::to_string(t); }
	template <> inline std::string ToString(const unsigned long long& t){ return std::to_string(t); }
	template <> inline std::string ToString(const float& t)             { return std::to_string(t); }
	template <> inline std::string ToString(const double& t)            { return std::to_string(t); }
	template <> inline std::string ToString(const unsigned short& t)    { return std::to_string(t); }
	template <> inline std::string ToString(const unsigned char& t)     { return std::to_string(t); }
	
	template <> inline std::string ToString(const std::string& t)       { return std::string(t); }
	template <> inline std::string ToString(const char* t)              { return std::string(t); }
	template <> inline std::string ToString(      void* t)
	{
		std::ostringstream oss;
		oss << "0x" << std::uppercase << std::hex << reinterpret_cast<long long>(t);
		return oss.str();
	}
	template <> inline std::string ToString(const std::wstring& t)
	{
		std::string s;
		for (wchar_t wc : t)
			s.push_back(static_cast<char>(wc)); // lossy: drops high bits
		return s;
	}
	template <> inline std::string ToString(const wchar_t* t) { return ToString(std::wstring(t)); }
}

export namespace TDD20
{
	struct AssertException : public std::exception
	{
		std::string message, file;
		int line;
		AssertException(const std::string& msg, int line, const std::string& file) : message(msg), line(line), file(file) {}
		virtual const char* what() const noexcept override { return message.c_str(); }
	};
	struct Assert
	{
		static void Fail(const std::string& msg="", std::source_location loc=std::source_location::current()) { throw AssertException(msg, loc.line(), loc.file_name()); }
		template<typename S, typename T> static void AreEqual(const S& expected, const T& actual, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			std::string Expected = Detail::ToString(expected);
			std::string Actual   = Detail::ToString(actual);
			if (Expected != Actual)
			{
				std::string msg = "Expected <";
				msg += Expected;
				msg += "> Actual <";
				msg += Actual;
				msg += ">";
				if (message.empty() == false) {
					msg += " - ";
					msg += message;
				}
				throw AssertException(msg, loc.line(), loc.file_name());
			}
		}
		template<typename S, typename T> static void AreNotEqual(const S& expected, const T& actual, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			std::string Actual = Detail::ToString(actual);
			if (Detail::ToString(expected) == Actual)
			{
				std::string msg = "Unexpected equality <";
				msg += Actual;
				msg += ">";
				if (message.empty() == false) {
					msg += " - ";
					msg += message;
				}
				throw AssertException(msg, loc.line(), loc.file_name());
			}
		}
		static void AreWithin(double expected, double actual, double tolerance, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			if (std::fabs(expected - actual) > tolerance) {
				std::string msg = "Expected <" + Detail::ToString(expected) + "> to be within <" + Detail::ToString(tolerance) + "> of <" + Detail::ToString(actual) + ">";
				if (message.empty() == false) {
					msg += " - ";
					msg += message;
				}
				throw AssertException(msg, loc.line(), loc.file_name());
			}
		}
		static void IsFalse(bool actual, const std::string& message="", std::source_location loc=std::source_location::current()) { AreEqual(false, actual, message, loc); }
		static void IsTrue (bool actual, const std::string& message="", std::source_location loc=std::source_location::current()) { AreEqual(true,  actual, message, loc); }
		template <typename E, typename L> static void ExpectingException(L l, const std::string& message=std::string(), std::source_location loc = std::source_location::current())
		{
			std::string m;
			if (message.empty() == false)
				m = std::string(" - ") + message;

			auto AddMoreText = [](const std::string& m) { return
#ifdef _CPPRTTI 
						  std::string("; was expecting exception of type '")
						+ std::string(typeid(E).name())
						+ std::string("'")
#else
						std::string(" (RTTI is turned off otherwise the expected exception type would be displayed here)")
#endif
						+ m;
				};

			try { l(); }
			catch (const E&) { return; }
			catch (...) { throw AssertException("exception of wrong type thrown" + AddMoreText(m), loc.line(), loc.file_name()); }
			throw               AssertException("no exception thrown"            + AddMoreText(m), loc.line(), loc.file_name());
		}
	};

	struct TestRegistrar
	{
		static auto& GetTests()
		{
			static std::vector<std::pair<std::string, std::function<void()>>> tests;
			return tests;
		}
		static std::pair<int, int> RunTests(auto&& matcher, auto&& out)
		{
			auto CreateVisualStudioCompatibleMessage = [](const std::string& name, const std::string& file, int line, const std::string& what) -> std::string
														{ return file + "(" + std::to_string(line) + ") : warning unit-test : \"" + name + "\" failed with: " + what + "\n"; };
			int passed = 0, failed = 0;
			for (auto& [name, func] : GetTests())
			{
				if (false == matcher.WantTest(name))
					continue;

				try {
					func();
					++passed;
					continue;
				}
				catch (const AssertException& e) { out << CreateVisualStudioCompatibleMessage(name, e.file, e.line, e.what()); }
				catch (const std::exception & e) { out << CreateVisualStudioCompatibleMessage(name, "?",    1,      e.what()); }
				catch (...)                      { out << CreateVisualStudioCompatibleMessage(name, "?",    1,      "unknown exception caught"); }
				++failed;
			}

			// summary
			std::ostringstream oss;
			oss << "\n" << failed << " failure(s) out of " << (passed + failed) << " test(s) run\n\n";
			out << oss.str();

			return {passed, failed};
		}
		TestRegistrar(std::pair<std::string, std::function<void()>>&& test)
		{
			GetTests().push_back(test);
		}
	};

	struct Test : TestRegistrar
	{
		Test(const std::string& name, std::function<void()> func) : TestRegistrar(std::pair{name, func}) {}
	};
}