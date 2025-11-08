import std;
import tdd20;

int main(int argc, char* argv[])
{
	if (argc > 1)
	if (argv[1] == std::string("-dump") ||
		argv[1] == std::string("/dump") )
	{
		struct Dumper { bool WantTest  (const std::string& name) const { std::cout << name << "\n"; return false; } };
		struct Noop   { void operator<<(const std::string&) {} };
		TDD20::TestRegistrar::RunTests(Dumper{}, Noop{});
		return 0;
	}

	class Matcher
	{
		auto MakeVector(int argc, char* argv[])
		{
			std::vector<std::string> v;
			for(int i=1; i<argc; ++i)
				v.push_back(argv[i]);
			return v;
		}
		const std::vector<std::string> args;
	public:
		Matcher(int argc, char* argv[]) : args(MakeVector(argc, argv)) {}
		bool WantTest(const std::string& name) const
		{
			return args.size() == 0 ? true : args.cend() != std::find(args.cbegin(), args.cend(), name);
		}
	};
	auto [passed, failed] = TDD20::TestRegistrar::RunTests(Matcher{argc, argv}, std::cout);
    return 0;
}
