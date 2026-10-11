import std;
import tdd20;

template<class Matcher, class Out> class Timing
{
	const Matcher matcher;
	Out& out;
	std::string testName;
	std::chrono::steady_clock::time_point start;

	void OutputTiming()
	{
		if (!testName.empty()) {
			out << std::format("{:>8} us: \"{}\"\n", std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count(), testName);
			testName.clear();
		}
	}
public:
	template<class... Args>	Timing(Out& out, Args&&... args) : matcher(std::forward<Args>(args)...), out(out) {}
	~Timing() { OutputTiming(); }
	bool WantTest(const std::string& name)
	{
		OutputTiming();
		if (!matcher.WantTest(name))
			return false;

		testName = name;
		start = std::chrono::steady_clock::now();
		return true;
	}
	void operator<<(const std::string& output)
	{
		OutputTiming();
		out << output;
	}
};

int main(int argc, char* argv[])
{
	if ((argc > 1) && (argv[1] == std::string("-dump") || argv[1] == std::string("-d") || argv[1] == std::string("--dump") || argv[1] == std::string("/dump")))
	{
		struct Dumper { bool WantTest  (const std::string& name) const { std::cout << name << "\n"; return false; } };
		struct Noop   { void operator<<(const std::string&) {} };
		TDD20::Test::RunTests(Dumper{}, Noop{});
		return 0;
	}

	class CmdLineMatcher
	{
		const std::vector<std::string> args;
	public:
		CmdLineMatcher(int argc, char* argv[]) : args(argv+1, argv+argc) {}
		bool WantTest(const std::string& name) const { return args.size() == 0 ? true : args.cend() != std::find(args.cbegin(), args.cend(), name); }
	};
	Timing<CmdLineMatcher, decltype(std::cout)> timer{std::cout, argc, argv};
	auto [passed, failed] = TDD20::Test::RunTests(timer, timer);
	timer << std::format("\n{} failure(s) out of {} test(s) run\n\n", failed, passed + failed);
    return 0;
}
