// Copyright © 2025 MiddleRaster
// Licensed under the MIT License
// See LICENSE file in the project root for full license information.

export module tdd20;

import std;

export namespace TDD20
{
	template<typename T> inline std::string ToString(const T&) { static_assert(sizeof(T) == 0, "test writer must write a specialization for this type"); }
	template<typename T> inline std::string ToString(T* t)
	{	// for generic pointer types, return hex value of address
		std::ostringstream oss;
		oss << "0x" << std::uppercase << std::hex << reinterpret_cast<std::uintptr_t>(t);
		return oss.str();
	}
	template<std::integral       T> inline std::string ToString(const T& t) { return std::to_string(t); }
	template<std::floating_point T> inline std::string ToString(const T& t) { return std::format("{:.15f}", t); } // N.B.: 15 digits of precision
	            inline std::string ToString(const         bool& t) { return t ? "true" : "false"; } // N.B.: an overload, not a specialization
	template <> inline std::string ToString(const  std::string& t) { return t; }
	template <> inline std::string ToString(const         char* t) { return std::string(t); }
	template <> inline std::string ToString(const std::wstring& t) { return std::accumulate(t.cbegin(), t.cend(), std::string(), [](auto s, wchar_t wc) { return s + static_cast<char>(wc); }); } //N.B.: lossy - drops high bits
	template <> inline std::string ToString(const      wchar_t* t) { return ToString(std::wstring(t)); }

	struct AssertException : public std::exception
	{
		const std::string message, file;
		const int line;
		AssertException(const std::string& msg, int line, const std::string& file) : message(msg), line(line), file(file) {}
		const char* what() const noexcept override { return message.c_str(); }
	};
	struct Assert
	{
		static void Fail(const std::string& msg="", std::source_location loc=std::source_location::current()) { throw AssertException(msg, loc.line(), loc.file_name()); }
		template<typename S, typename T> static void AreEqual(const S& expected, const T& actual, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			std::string Expected{ToString(expected)}, Actual{ToString(actual)};
			if (Expected != Actual)
				throw AssertException(std::format("Expected <{}> Actual <{}>{}", Expected, Actual, message.empty() ? "" : " - " + message), loc.line(), loc.file_name());
		}
		template<typename S, typename T> static void AreNotEqual(const S& expected, const T& actual, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			std::string Actual = ToString(actual);
			if (ToString(expected) == Actual)
				throw AssertException(std::format("Unexpected equality <{}>{}", Actual, message.empty() ? "" : " - " + message), loc.line(), loc.file_name());
		}
		static void AreWithin(double expected, double actual, double tolerance, const std::string& message="", std::source_location loc=std::source_location::current())
		{
			if (std::fabs(expected - actual) > tolerance)
				throw AssertException(std::format("Expected <{}> to be within <{}> of <{}>{}", ToString(expected), ToString(tolerance), ToString(actual), message.empty() ? "" : " - " + message), loc.line(), loc.file_name());
		}
		static void IsFalse(bool actual, const std::string& message="", std::source_location loc=std::source_location::current()) { AreEqual(false, actual, message, loc); }
		static void IsTrue (bool actual, const std::string& message="", std::source_location loc=std::source_location::current()) { AreEqual(true,  actual, message, loc); }
		template <typename E, typename L> static void ExpectingException(L l, const std::string& message=std::string(), std::source_location loc=std::source_location::current())
		{
			try { l(); }
			catch (const E&) { return; }
			catch (...) { throw AssertException(std::format("exception of wrong type thrown; was expecting exception of type '{}'{}", typeid(E).name(), message.empty() ? "" : " - " + message), loc.line(), loc.file_name()); }
			throw               AssertException(std::format(           "no exception thrown; was expecting exception of type '{}'{}", typeid(E).name(), message.empty() ? "" : " - " + message), loc.line(), loc.file_name());
		}
	};

	struct Test
	{
		static auto& GetTests()
		{
			static std::vector<std::pair<std::string, std::function<void()>>> tests;
			return tests;
		}
		static std::pair<int, int> RunTests(auto&& matcher, auto&& out)
		{
			auto CreateVisualStudioCompatibleMessage = [](const std::string& name, const std::string& file, int line, const std::string& what) -> std::string
														{ return std::format("{}({}) : warning unit-test: \"{}\" failed with: {}\n", file, line, name, what ); };
			int passed = 0, failed = 0;
			for (auto& [name, func] : GetTests()) {
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
			out << std::format("\n{} failure(s) out of {} test(s) run\n\n", failed, passed + failed); // output summary
			return {passed, failed};
		}
		Test(const std::string& name, std::function<void()> func) { GetTests().push_back(std::pair{name, func}); }
	};
}