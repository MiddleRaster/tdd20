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


// a simple combination RSpec-style with Gherkin syntax BDD test harness

namespace BoBW
{
	struct Specification
	{
		template <typename Node> Specification(Node node) { node(std::string{}, []() { return std::tuple<>{}; }); }
	};
	template <typename F, typename Tuple> decltype(auto) InvokeWithSingleParamOrTuple(F&& f, Tuple& tuple)
	{
		if constexpr (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 0)
			return f();
		else if constexpr (std::tuple_size_v<std::remove_cvref_t<Tuple>> == 1)
			return f(std::get<0>(tuple));
		else
			return f(tuple);
	}
	template <typename Ancestor, typename Setup> auto Compose(Ancestor&& ancestor, Setup&& setup)
	{
		return [ancestor = std::forward<Ancestor>(ancestor), setup = std::forward<Setup>(setup)]() mutable
			{
				auto result = ancestor();
				using Result = decltype(InvokeWithSingleParamOrTuple(setup, result));
				if constexpr (std::is_void_v<Result>) {
					InvokeWithSingleParamOrTuple(setup, result);
					return result;
				} else {
					auto added = InvokeWithSingleParamOrTuple(setup, result);
					return std::tuple_cat(std::move(result), std::tuple<std::remove_cvref_t<Result>>{std::move(added)});
				}
			};
	}
	template <typename Ancestor>                       void visitChildren(const std::string& path, Ancestor ancestor                        ) { TDD20::Test(path.substr(1), [ancestor]() mutable { ancestor(); }); }
	template <typename Ancestor, typename... Children> void visitChildren(const std::string& path, Ancestor ancestor, Children&&... children) { (children(path, ancestor), ...); }
	template <typename Setup, typename... Children> auto When(std::string name, Setup&& setup, Children&&... children)
	{
		return [name = std::move(name), setup = std::forward<Setup>(setup), ...children = std::forward<Children>(children)](std::string path, auto ancestor) mutable
			{
				path += " " + name;
				visitChildren(path, Compose(ancestor, setup), children...);
			};
	}
	template <typename Test                       > auto Then (std::string name, Test && test)                          { return When(std::move(name), std::forward<Test>(test)); }
	template <typename Setup, typename... Children> auto Given(std::string name, Setup&& setup, Children&&... children) { return When(std::move(name), std::forward<Setup>(setup), std::forward<Children>(children)...); }
}

namespace BankSteps
{
	inline auto Deposit (double amount) { return [amount](auto& bank) { bank.Deposit(amount); }; }
	inline auto Withdraw(double amount) { return [amount](auto& bank) { bank.Withdraw(amount); }; }
}

using namespace BoBW;

Specification specs[] = {
	Given("[BoBW] Bank", []() { return Bank{}; },
		Then("starts with a zero balance"            , [](auto& bank) { Assert::AreWithin(0.00, bank.Balance(), 0.0001); }),
		When("with sufficient funds"                 , BankSteps::Deposit(100.00),
			When("withdrawal"                        , BankSteps::Withdraw(50.00),
				Then("the balance is reduced"        , [](auto& bank) { Assert::IsTrue (100.00 > bank.Balance(), "balance should be less than 100.0"); }),
				Then("the balance is nonnegative"    , [](auto& bank) { Assert::IsTrue  (0.00 <= bank.Balance(), "balance cannot go negative"); }),
				Then("the balance is $50.00"         , [](auto& bank) { Assert::AreWithin(50.00, bank.Balance(), 0.0001); })
			)
		),
		When("account belonging to MiddleRaster"     , [](auto&) { return "MiddleRaster"; }, // a second return value: use tuple from here on
			When("withdrawal with insufficient funds", [](auto& tuple) { auto& [bank,customer] = tuple; bank.Deposit(0.01); },
				Then("is the same customer"          , [](auto& tuple) { auto& [bank,customer] = tuple; Assert::AreEqual("MiddleRaster", customer, "wrong customer!"); }),
				Then("raises an error"               , [](auto& tuple) { auto& [bank,customer] = tuple; Assert::ExpectingException<std::invalid_argument>([&bank]() { bank.Withdraw(50.00); }, "cannot withdraw more than balance"); })
			)
		)
	),
	Given("[BoBW] returning void", []() {},	// Given's Setup returns void
		Then("is ok"             , []() {})
	),
	Given("[BoBW] impossible opening of account with a negative balance", []() { Assert::ExpectingException<std::invalid_argument>([]() { return Bank{ -0.001 }; }, "cannot open account with negative balance"); }
	),
	Given("[BoBW] some customer's name", [](              ) { return "MiddleRaster"; },
		When("is not pseudonym"        , [](auto& customer) { Assert::ExpectingException<TDD20::AssertException>([&customer]() { Assert::AreNotEqual("MiddleRaster", customer); }); })
	),

	Given("[BoBW] a national bank"               , [](           ) { return "Wells-Fargo"; },
		Given("and a bank account"               , [](auto&      ) { return Bank{}; },
			Then("the account is empty"          , [](auto& tuple) { auto& [bank, account] = tuple; Assert::AreWithin(0.00, account.Balance(), 0.0001); }),
		  	Given("a customer"                   , [](auto&      ) { return "Mr. MiddleRaster"; },
				When("he deposits $100.00"       , [](auto& tuple) {                           std::get<1>   (tuple).Deposit(100.00); },
					Then("the balance is $100.00", [](auto& tuple) { Assert::AreWithin(100.00, std::get<Bank>(tuple).Balance(), 0.0001); })
				)
			)
		)
	),
	Given("[BoBW] a well-known bank"                                        , []() { return "Wells Fargo"; },
		When("there's a customer"                                           , [](auto&) { return "Mr. MiddleRaster"; },
			Given("with a savings account"                                  , [](auto&) { return Bank{}; },
				Given("and a checking account"                              , [](auto&) { return Bank{}; },
					When("he deposits $100 into savings"                    , [](auto& tuple) { auto& [bank, customer, savings, checking] = tuple; savings.Deposit(100.00); },
						When("he transfers $50 from savings into checking"  , [](auto& tuple) { auto& [bank, customer, savings, checking] = tuple; 
	                                                                                            savings.Withdraw(50.00);
                                                                                                checking.Deposit(50.00); },
							Then("he has $50 in savings and $50 in checking", [](auto& tuple) { auto& [bank, customer, savings, checking] = tuple;
																								Assert::AreWithin(50.00,  savings.Balance(), 0.0001);
																								Assert::AreWithin(50.00, checking.Balance(), 0.0001);
																								Assert::AreEqual("Wells Fargo"     , bank);
																								Assert::AreEqual("Mr. MiddleRaster", customer);
								}
							)
						)
					)
				)
			)
		)
	),
};
