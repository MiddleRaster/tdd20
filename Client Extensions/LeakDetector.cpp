#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

import std;
import tdd20;

template<class Matcher, class Out> class LeakDetector
{
    const Matcher matcher;
    Out& out;
    std::string testName;
    _CrtMemState start;
    TDD20::Test lastTest{"LeakDetector's dummy last test", []() {}};

    void OutputLeaks()
    {
        if (!testName.empty()) {
            _CrtMemState end, diff;
            _CrtMemCheckpoint(&end);

            if (_CrtMemDifference(&diff, &start, &end))
            {
                long bytesLeaked = 0;
                for (int i=0; i<_MAX_BLOCKS; ++i)
                    bytesLeaked += static_cast<long>(diff.lSizes[i]);
                if (bytesLeaked != 0)
                    out << std::format("Bytes leaked during test: \"{}\": {}\n", testName, bytesLeaked);
            }
            testName.clear();
        }
    }
public:
    template<class... Args> LeakDetector(Out& out, Args&&... args) : matcher(std::forward<Args>(args)...), out(out) {}

    bool WantTest(const std::string& name)
    {
        OutputLeaks();
        if (!matcher.WantTest(name))
            return false;

        testName = name;
        _CrtMemCheckpoint(&start);
        return true;
    }
};

TDD20::Test aLeakingTest("a leaking test", []() { new char[5000]; });

int main(int argc, char* argv[])
{
    if ((argc > 1) && (argv[1] == std::string("-dump") || argv[1] == std::string("-d") || argv[1] == std::string("--dump") || argv[1] == std::string("/dump")))
    {
        struct Dumper { bool WantTest(const std::string& name) const { std::cout << name << "\n"; return false; } };
        struct Noop { void operator<<(const std::string&) {} };
        TDD20::Test::RunTests(Dumper{}, Noop{});
        return 0;
    }

    class CmdLineMatcher
    {
        const std::vector<std::string> args;
    public:
        CmdLineMatcher(int argc, char* argv[]) : args(argv + 1, argv + argc) {}
        bool WantTest(const std::string& name) const { return args.size() == 0 ? true : args.cend() != std::find(args.cbegin(), args.cend(), name); }
    };
    LeakDetector<CmdLineMatcher, decltype(std::cout)> leakDetector{std::cout, argc, argv};
    auto [passed, failed] = TDD20::Test::RunTests(leakDetector, std::cout);
    std::cout << std::format("\n{} failure(s) out of {} test(s) run\n\n", failed, passed + failed);
    return 0;
}

