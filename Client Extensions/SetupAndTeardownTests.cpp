import tdd20;
import std;
using namespace TDD20;


// how to write test fixtures (aka, setup and teardown methods)

//#include <scope> // doesn't exist yet, so roll our own minimal scope_exit
template<typename F> struct scope_exit
{
	F f;
	explicit scope_exit(F&& f) noexcept : f(f) {}
	~scope_exit() { f(); } // if it throws while in a throw, it'll terminate, but it's better than swallowing an exception silently (for a unit testing harness)
};

// a test suite where a file is writable on entry to each test lambda and is cleaned up after exit
class FileIOTest : public Test
{
    static constexpr char TestFile[] = "TestData.txt";
    std::ofstream file;
private:
    void TestMethodInitialize() { file.open(TestFile); }
    void TestMethodCleanup()
    {
        if (file.is_open())
            file.close();
        std::filesystem::remove(TestFile);
    }
public:
    FileIOTest(const std::string& name, std::function<void(std::ofstream& file)> func)
        : Test(name, [this, func]() {
                                        TestMethodInitialize();
                                        scope_exit always([this]() { TestMethodCleanup(); });
                                        func(file);
                                    })
    {}
} fileIOTests[] = {
    {"File IO test 1: write \"Hello, world!\"",[](auto& file)
        {
            file << "Hello, world!\n";
            file.flush();
            Assert::IsTrue(file.good());
        }
    },
    {"File IO test 2: write several lines",[](auto& file)
        {
            file << "Line 1\n";
            file << "Line 2\n";
            file << "Line 3\n";
            file.flush();
            Assert::IsTrue(file.good());
        }
    },
};
