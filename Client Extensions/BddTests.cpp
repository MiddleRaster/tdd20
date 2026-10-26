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

template <typename GivenT, typename WhenT, typename ThenT> struct BDDTest
{
	std::string name;
	GivenT given;
	WhenT  when;
	ThenT  then;

	template <typename NewGiven> auto Given(const std::string&, NewGiven newGiven) const { return BDDTest<NewGiven, WhenT, ThenT>{name, newGiven, when, then}; }
	template <typename NewWhen > auto When (const std::string&, NewWhen  newWhen ) const { return BDDTest<GivenT, NewWhen, ThenT>{name, given, newWhen, then}; }
	template <typename NewThen > auto Then (const std::string&, NewThen  newThen ) const { return BDDTest<GivenT, WhenT, NewThen>{name, given, when, newThen}; }
};

struct Fluent
{
	inline static constexpr auto DefaultGiven = []() { return 0; };
	inline static constexpr auto DefaultWhen  = [](      auto&) {};
	inline static constexpr auto DefaultThen  = [](const auto&) {};
	inline static auto BDD(std::string name)
	{
		return BDDTest<decltype(DefaultGiven), decltype(DefaultWhen), decltype(DefaultThen)>{name, DefaultGiven, DefaultWhen, DefaultThen};
	}
};

class BDDTestSuite : public Test
{
public:
	template <typename Given, typename When, typename Then> BDDTestSuite(BDDTest<Given, When, Then> def)
		: Test(def.name, [given = std::move(def.given), when = std::move(def.when), then = std::move(def.then)]()
			{
				auto context = given();
				when(context);
				then(context);
			})
	{}
} bddTests[] =
{
	BDDTest{
		.name  = "can open account with positive funds",
		.given = [](                ) { return Bank{0.01}; },
		.when  = [](      auto&     ) { },
		.then  = [](const auto& bank) { Assert::AreWithin(.01, bank.Balance(), .0001); }
	},
	BDDTest{
		.name  = "cannot open account with negative funds",
		.given = [](           ) { Assert::ExpectingException<std::invalid_argument>([](){ return Bank{-0.01}; }, "cannot open account with negative balance"); return 0; },
		.when  = [](      auto&) { },
		.then  = [](const auto&) { }
	},
	BDDTest {
		.name  = "depositing money increases balance",
		.given = [](                ) { return Bank{100.0}; },
		.when  = [](      auto& bank) { bank.Deposit(50.0); },
		.then  = [](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }
	},
	BDDTest {
		.name  = "deposit cannot be negative",
		.given = [](                ) { return Bank{100.0}; },
		.when  = [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Deposit(-0.01); }, "cannot deposit negative amount"); },
		.then  = [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); }
	},
	BDDTest {
		.name  = "withdrawing money decreases balance",
		.given = [](                ) { return Bank{100.0}; },
		.when  = [](      auto& bank) { bank.Withdraw(50.0); },
		.then  = [](const auto& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); }
	},
	BDDTest {
		.name  = "cannot withdraw more than balance",
		.given = [](                ) { return Bank{100.0}; },
		.when  = [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(100.01); }, "cannot withdraw more than balance"); },
		.then  = [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); }
	},
	BDDTest {
		.name  = "cannot withdraw negative amount",
		.given = [](                ) { return Bank{100.0}; },
		.when  = [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(-0.01); }, "cannot withdraw negative amount"); },
		.then  = [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); }
	},

	Fluent::BDD("as a bank customer, I can open an account and deposit money into it")
		.Given("an open bank account containing $100.00", [](                ) { return Bank{100.0}; })
		.When ("I deposit $50.00"                       , [](      auto& bank) { bank.Deposit(50.0); })
		.Then ("the balance should be $150.00"          , [](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }),

	Fluent::BDD("as a bank customer, I can open an account and withdraw some of the funds")
		.Given("an open account"      , [](                ) { return Bank{100.0}; })
		.When ("I withdraw some funds", [](      auto& bank) { bank.Withdraw(75.0);})
		.Then ("I have $25 remaining" , [](const auto& bank) { Assert::AreWithin(25.0, bank.Balance(), .0001); }),
};
