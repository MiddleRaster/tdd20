import std;
import tdd20;
using namespace TDD20;

class RetryingTest : public Test
{
public:
	RetryingTest(const std::string& name, std::function<void()> test, int maxRetries = 3)
		: Test(name, [test = std::move(test), maxRetries]() {
			for (int attempt = 0; ; ++attempt) {
				try {
					test();
					return;
				}
				catch (...) {
					if (attempt >= maxRetries)
						throw;
				}
			}
		})
	{}
} retryTests[] = 
{
	{"retry test", [](){ /*Assert::Fail("fail every time");*/ }, 3},
};
