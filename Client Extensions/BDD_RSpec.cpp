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
				auto result  = ancestor();
				using Result = std::invoke_result_t<decltype(setup)&, decltype(result)&>;
				if constexpr (std::is_void_v<Result>) {
					std::invoke(setup, result);
					return result;
				} else {
					auto added = std::invoke(setup, result);
					return std::tuple_cat(std::move(result), std::tuple<std::remove_cvref_t<Result>>{std::move(added)});
				}
			};
	}
	template <typename Test> auto It(std::string name, Test&& test)
	{
		return [name=std::move(name), test=std::forward<Test>(test)](std::string path, auto ancestor) mutable
			{
				TDD20::Test(path + " " + name, [ancestor, test=std::move(test)]() mutable
					{
						auto result = ancestor();
						std::invoke(test, result);
					});
				return std::size_t{1}; // for count in Describe and Context
			};
	}
	template <typename Setup, typename... Children> auto Context(std::string name, Setup&& setup, Children&&... children)
	{
		return [name=std::move(name), setup=std::forward<Setup>(setup), ...children=std::forward<Children>(children)](std::string path, auto ancestor) mutable
			{
				auto combined = Compose(ancestor, setup);
				path += " " + name;
				auto count = (std::size_t{0} + ... + children(path, combined));
				if (count == 0) {
					TDD20::Test(path, [combined]() mutable { combined(); });
					count = 1;
				}
				return count;
			};
	}
	template <typename Setup, typename... Children> Specification Describe(std::string name, Setup&& setup, Children&&... children)
	{
		auto ancestor = [setup=std::forward<Setup>(setup)]() mutable
			{
				if constexpr (std::is_void_v<std::invoke_result_t<decltype(setup)&>>) {
					std::invoke(setup);
					return std::tuple{};
				} else
					return std::tuple{std::invoke(setup)};
			};
		auto count = (std::size_t{0} + ... + children(name, ancestor));
		if (count == 0)
			TDD20::Test(name, [ancestor]() mutable { ancestor(); });

		return {};
	}
}

namespace BankSteps
{
	inline auto Deposit (double amount) { return [amount](auto& tuple) { auto& [bank]=tuple; bank.Deposit (amount); }; }
	inline auto Withdraw(double amount) { return [amount](auto& tuple) { auto& [bank]=tuple; bank.Withdraw(amount); }; }
}

using namespace RSpec;

Specification specs[] = {
	Describe("Bank", []() { return Bank{}; },
		It("starts with a zero balance"             , [](auto& tuple) { auto& [bank]=tuple; Assert::AreWithin( 0.00, bank.Balance(), 0.0001); }),
		Context("with sufficient funds"             , BankSteps::Deposit(100.00),
			Context("withdrawal"                    , BankSteps::Withdraw(50.00),
				It("reduces the balance"            , [](auto& tuple) { auto& [bank]=tuple; Assert::IsTrue (100.00 > bank.Balance(), "balance should be less than 100.0"); }),
				It("the balance is nonnegative"     , [](auto& tuple) { auto& [bank]=tuple; Assert::IsTrue ( 0.00 <= bank.Balance(), "balance cannot go negative"); }),
				It("the balance is $50.00"          , [](auto& tuple) { auto& [bank]=tuple; Assert::AreWithin(50.00, bank.Balance(), 0.0001); })
			)
		),
		Context("account belonging to MiddleRaster"     , [](auto&      ) { return "MiddleRaster"; },
			Context("withdrawal with insufficient funds", [](auto& tuple) { auto& [bank,customer]=tuple; bank.Deposit(0.01); },
				It("is the same customer"               , [](auto& tuple) { auto& [bank,customer]=tuple; Assert::AreEqual("MiddleRaster", customer, "wrong customer!"); }),
				It("raises an error"                    , [](auto& tuple) { auto& [bank,customer]=tuple; Assert::ExpectingException<std::invalid_argument>([&bank]() { bank.Withdraw(50.00); }, "cannot withdraw more than balance"); })
			)
		)
	),
	Describe("returning void", [](){},	// Describe's Setup returns void
		It("is ok",  [](auto&) {})		// tuple is empty
	),
	Describe("impossible opening of account with a negative balance", []() { Assert::ExpectingException<std::invalid_argument>([]() { return Bank{-0.001}; }, "cannot open account with negative balance"); }
	),
	Describe("some customer"      , []() { return "MiddleRaster"; },
		Context("is not pseudonym", [](auto& tuple) { auto& [customer] = tuple; Assert::ExpectingException<TDD20::AssertException>([&customer]() { Assert::AreNotEqual("MiddleRaster", customer); }); })),
};
