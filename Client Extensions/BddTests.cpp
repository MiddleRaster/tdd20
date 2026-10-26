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


// how to write BDD tests

template <typename GivenTuple, typename WhenTuple, typename ThenTuple> struct BDDTest
{
	std::string name;
	GivenTuple givens;
	 WhenTuple whens;
	 ThenTuple thens;

	template <typename NewGiven> auto Given(const std::string&, NewGiven newGiven) const { return Given(newGiven); }
	template <typename NewGiven> auto Given(NewGiven newGiven) const
	{
		auto newGivens = std::tuple_cat(givens, std::tuple{std::move(newGiven)});
		return BDDTest<decltype(newGivens), WhenTuple, ThenTuple>{name, std::move(newGivens), whens, thens};
	}
	template <typename NewWhen> auto When(const std::string&, NewWhen newWhen) const { return When(newWhen); }
	template <typename NewWhen> auto When(NewWhen newWhen) const
	{
		auto newWhens = std::tuple_cat(whens, std::tuple{std::move(newWhen)});
		return BDDTest<GivenTuple, decltype(newWhens), ThenTuple>{name, givens, std::move(newWhens), thens};
	}
	template <typename NewThen> auto Then(const std::string&, NewThen newThen) const { return Then(newThen); }
	template <typename NewThen> auto Then(NewThen newThen) const
	{
		auto newThens = std::tuple_cat(thens, std::tuple{std::move(newThen)});
		return BDDTest<GivenTuple, WhenTuple, decltype(newThens)>{name, givens, whens, std::move(newThens)};
	}
};
struct Fluent
{
	inline static auto BDD(std::string name) { return BDDTest<std::tuple<>, std::tuple<>, std::tuple<>>{std::move(name),{},{},{}}; }
};

struct BDDTestSuite : public Test
{
	template <typename GivenTuple, typename WhenTuple, typename ThenTuple> BDDTestSuite(BDDTest<GivenTuple, WhenTuple, ThenTuple> def)
		: Test(def.name, [givens = std::move(def.givens), whens = std::move(def.whens), thens = std::move(def.thens)]()
			{
				auto tuple =
				std::apply([ ](auto const&... given){ return std::tuple{given() ...}; }, givens);
				std::apply([&](auto const&... when) { (std::apply(when, tuple), ...); }, whens);
				std::apply([&](auto const&... then) { (std::apply(then, tuple), ...); }, thens);
			})
	{}
} bddTests[] =
{
	BDDTest{
		.name   = "can open account with positive funds",
		.givens = std::tuple{[](                ) { return Bank{0.01}; } },
		.whens  = std::tuple{[](      auto&     ) { } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(.01, bank.Balance(), .0001); } }
	},
	BDDTest{
		.name   = "cannot open account with negative funds",
		.givens = std::tuple{[](           ) { Assert::ExpectingException<std::invalid_argument>([](){ return Bank{-0.01}; }, "cannot open account with negative balance"); return 0; } },
		.whens  = std::tuple{[](      auto&) { } },
		.thens  = std::tuple{[](const auto&) { } }
	},
	BDDTest {
		.name   = "depositing money increases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      auto& bank) { bank.Deposit(50.0); } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "deposit cannot be negative",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Deposit(-0.01); }, "cannot deposit negative amount"); } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "withdrawing money decreases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      auto& bank) { bank.Withdraw(50.0); } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "cannot withdraw more than balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(100.01); }, "cannot withdraw more than balance"); } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "cannot withdraw negative amount",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(-0.01); }, "cannot withdraw negative amount"); } },
		.thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},

	BDDTest {
		.name   = "withdrawing money decreases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
	    .whens  = std::tuple{[](      auto& bank) { bank.Withdraw(50.0); } },
	    .thens  = std::tuple{[](const auto& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); } }
	},

	Fluent::BDD("as a bank customer, I can open an account and deposit money into it")
		.Given("an open bank account containing $100.00", [](                ) { return Bank{100.0}; })
		.When ("I deposit $50.00"                       , [](      auto& bank) { bank.Deposit(50.0); })
		.Then ("the balance should be $150.00"          , [](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }),

	Fluent::BDD("as a bank customer, I can open an account and withdraw some of the funds")
		.Given("an open account"      , [](                ) { return Bank{100.0}; })
		.When ("I withdraw some funds", [](      auto& bank) { bank.Withdraw(75.0);})
		.Then ("I have $25 remaining" , [](const auto& bank) { Assert::AreWithin(25.0, bank.Balance(), .0001); }),

	Fluent::BDD("customer deposits money")
		.Given([](                                      ) { return "Bill"; })
		.Given([](                                      ) { return Bank{100.0}; })
		.When ([](      auto&,                auto& bank) { bank.Deposit(25.0);})
		.When ([](      auto&,                auto& bank) { bank.Deposit(25.0);})
		.Then ([](const auto& customer, const auto&     ) { Assert::AreEqual("Bill", customer); })
		.Then ([](const auto&,          const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }),

};
