import std;
import tdd20;
using namespace TDD20;

// system under test
class Bank
{
	double balance;
	void CheckAmount(double amount, const std::string& message) { if (amount < 0.0) throw std::invalid_argument(message); }
public:
	Bank(double balance=0.0) : balance(balance) { CheckAmount(balance, "cannot open account with negative balance"); }
	void Deposit(double deposit)
	{
		CheckAmount(deposit, "cannot deposit a negative amount");
		balance += deposit;
	}
	void Withdraw(double withdrawal)
	{
		CheckAmount(        withdrawal, "cannot withdraw a negative amount");
		CheckAmount(balance-withdrawal, "cannot withdraw more than balance");
		balance -= withdrawal;
	}
	double Balance() const { return balance; }
};


// a simple RSpec-style BDD test harness

namespace RSpec
{
	struct Specification {};
	template <typename Ancestor, typename Setup> auto Compose(Ancestor&& ancestor, Setup&& setup)
	{
		return [ancestor=std::forward<Ancestor>(ancestor), setup=std::forward<Setup>(setup)]() mutable
			{
				auto result = ancestor();
				if constexpr (std::is_void_v<std::invoke_result_t<Setup&, decltype(result)&>>) {
					std::invoke(setup, result);
					return result;
				} else
					return std::invoke(setup, result);
			};
	}
	template <typename Test> auto It(std::string name, Test&& test)
	{
		return [name=std::move(name), test=std::forward<Test>(test)](std::string path, auto ancestor) mutable
			{
				TDD20::Test(path + " " + name, [ancestor, test]() mutable
					{
						auto result = ancestor();
						std::invoke(test, result);
					});
			};
	}
	template<typename Setup, typename... Children> auto Context(std::string name, Setup setup, Children&&... children)
	{
		return [name=std::move(name), setup=std::move(setup), ...children=std::forward<Children>(children)](std::string path, auto ancestor) mutable
			{
				auto combined = Compose(ancestor, setup);
				path += " " + name;
				(children(path, combined), ...);
			};
	}
	template <typename Setup, typename... Children> Specification Describe(std::string name, Setup&& setup, Children&&... children)
	{
		auto ancestor = [setup=std::forward<Setup>(setup)]() mutable { return std::invoke(setup); };
		(children(name, ancestor), ...);
		return {};
	}
}

namespace BankSteps
{
	inline auto Deposit (double amount) { return [amount](Bank& bank) { bank.Deposit (amount); }; }
	inline auto Withdraw(double amount) { return [amount](Bank& bank) { bank.Withdraw(amount); }; }
}

using namespace RSpec;

auto spec = 
	Describe("Bank", []() { return Bank{}; },
		It("starts with a zero balance"             , [](Bank& bank) { Assert::AreWithin( 0.00, bank.Balance(), 0.0001); }),
		Context("with sufficient funds"             , BankSteps::Deposit(100.00),
			Context("withdrawal"                    , BankSteps::Withdraw(50.00),
				It("reduces the balance"            , [](Bank& bank) { Assert::IsTrue (100.00 > bank.Balance(), "balance should be less than 100.0"); }),
				It("the balance is nonnegative"     , [](Bank& bank) { Assert::IsTrue ( 0.00 <= bank.Balance(), "balance cannot go negative"); }),
				It("the balance is $50.00"          , [](Bank& bank) { Assert::AreWithin(50.00, bank.Balance(), 0.0001); })
			)
		),
		Context("withdrawal with insufficient funds", [](Bank& bank) { bank.Deposit(0.01); },
			It("raises an error"                    , [](Bank& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { bank.Withdraw(50.00); }, "cannot withdraw more than balance"); })
		)
	);
