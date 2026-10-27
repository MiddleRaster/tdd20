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


// a simple BDD test harness


// step tags: each wraps one lambda and knows how to run itself, then hand the possibly accumulated arguments onward
template <typename Lambda> struct GivenStep
{
	Lambda body;
	template <typename Args, typename Next> void Run(const Args& args, const Next& next) const
	{
		if constexpr (std::is_void_v<std::invoke_result_t<const Lambda&>>) {
			body();
			next(args);
		} else {
			auto&& value = body();	// prvalue initializes the object in place: no move, no copy
			next(std::tuple_cat(args, std::tuple<std::remove_reference_t<decltype(value)>&>{value}));
		}
	}
};
template <typename Lambda> struct WhenStep
{
	Lambda body;
	template <typename Args, typename Next> void Run(const Args& args, const Next& next) const
	{
		std::apply(body, args);
		next(args);
	}
};
template <typename Lambda> struct ThenStep
{
	Lambda body;
	template <typename Args, typename Next> void Run(const Args& args, const Next& next) const
	{
		std::apply([&](auto&... values) { body(std::as_const(values)...); }, args);
		next(args);
	}
};
template <std::size_t Index=0, typename StepTuple, typename Args=std::tuple<>> void RecursivelyRunSteps(const StepTuple& steps, const Args& args={})
{	// runs the steps in declaration order; args holds references to the values of the Givens run so far
	if constexpr (Index < std::tuple_size_v<StepTuple>)
		std::get<Index>(steps).Run(args, [&](const auto& nextArgs) { RecursivelyRunSteps<Index+1>(steps, nextArgs); });
}

template <typename StepTuple> struct BDDTest
{
	std::string name;
	StepTuple   steps;

	template <typename Lambda> auto Given(const std::string&, Lambda lambda) const { return Given (          std::move(lambda) ); }
	template <typename Lambda> auto Given(                    Lambda lambda) const { return Append(GivenStep{std::move(lambda)}); }
	template <typename Lambda> auto When (const std::string&, Lambda lambda) const { return When  (          std::move(lambda) ); }
	template <typename Lambda> auto When (                    Lambda lambda) const { return Append(WhenStep {std::move(lambda)}); }
	template <typename Lambda> auto Then (const std::string&, Lambda lambda) const { return Then  (          std::move(lambda) ); }
	template <typename Lambda> auto Then (                    Lambda lambda) const { return Append(ThenStep {std::move(lambda)}); }
private:
	template <typename Step> auto Append(Step step) const
	{
		auto newSteps = std::tuple_cat(steps, std::tuple{std::move(step)});
		return BDDTest<decltype(newSteps)>{name, std::move(newSteps)};
	}
};
struct Fluent
{
	inline static auto BDD(std::string name) { return BDDTest<std::tuple<>>{std::move(name), {}}; }
};

struct BDDTestSuite : public Test
{
	template <typename StepTuple> BDDTestSuite(BDDTest<StepTuple> def) : Test(def.name, [steps = std::move(def.steps)]() { RecursivelyRunSteps(steps); }) {}
} bddTests[] =
{
	BDDTest { // low-level guts
		.name  = "can open account with positive funds",
		.steps = std::tuple{ GivenStep{[](                ) { return Bank{0.01}; }},
		                     WhenStep {[](      auto&     ) { }},
		                     ThenStep {[](const auto& bank) { Assert::AreWithin(.01, bank.Balance(), .0001); }} }
	},
	BDDTest {
		.name  = "can deposit money into account",
		.steps = std::tuple{ GivenStep{[](                ) { return Bank{0.01}; }},
		                     WhenStep {[](      auto& bank) { bank.Deposit(0.02); }},
		                     WhenStep {[](      auto& bank) { bank.Deposit(0.03); }},
		                     ThenStep {[](const auto& bank) { Assert::AreWithin(.06, bank.Balance(), .0001); }} }
	},


	// the high-level fluent API
	Fluent::BDD("cannot open account with negative funds")
		.Given("cannot open an account with a negative balance", []() { Assert::ExpectingException<std::invalid_argument>([]() { return Bank{-0.01}; }, "cannot open account with negative balance"); })
	,
	Fluent::BDD("depositing money increases balance")
		.Given("a bank account with a $100.00 balance", [](                ) { return Bank{ 100.0 }; })
		.When ("I deposit $50.00 more"                , [](      auto& bank) { bank.Deposit(50.0); })
		.Then ("I have $150.00"                       , [](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("deposit cannot be negative")
		.Given("a bank account with a $100.00 balance" , [](                ) { return Bank{100.0}; })
		.When ("I try to deposit -.01, it doesn't work", [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Deposit(-0.01); }, "cannot deposit negative amount"); })
		.Then ("I still have the $100.00 balance"      , [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("withdrawing money decreases balance")
		.Given("a bank account with a $100.00 balance", [](                ) { return Bank{100.0}; })
		.When ("I withdraw $50.00"                    , [](      auto& bank) { bank.Withdraw(50.0); })
		.Then ("I have a $50.00 balance remaining",     [](const auto& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("cannot withdraw more than balance")
		.Given("a bank account with a $100.00 balance"                     , [](                ) { return Bank{100.0}; })
		.When ("I try to withdraw more than my balance, which doesn't work", [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(100.01); }, "cannot withdraw more than balance"); })
		.Then ("I still have the $100.00 balance"                          , [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("cannot withdraw negative amount")
		.Given("a bank account with a $100.00 balance"                  , [](                ) { return Bank{ 100.0 }; })
		.When ("I try to withdraw a negative amount, which doesn't work", [](      auto& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(-0.01); }, "cannot withdraw negative amount"); })
		.Then ("I still have the $100.00 balance"                       , [](const auto& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("withdrawing money decreases balance")
		.Given("a bank account with a $100.00 balance", [](                ) { return Bank{ 100.0 }; })
		.When ("I withdraw $50.00"                    , [](      auto& bank) { bank.Withdraw(50.0); })
		.Then ("I have a balance of $50.00"           , [](const auto& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("as a bank customer, I can open an account and deposit money into it")
		.Given("an open bank account containing $100.00", [](                ) { return Bank{100.0}; })
		.When ("I deposit $50.00"                       , [](      auto& bank) { bank.Deposit(50.0); })
		.Then ("the balance should be $150.00"          , [](const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("as a bank customer, I can open an account and withdraw some of the funds")
		.Given("an open account"      , [](                ) { return Bank{100.0}; })
		.When ("I withdraw some funds", [](      auto& bank) { bank.Withdraw(75.0);})
		.Then ("I have $25 remaining" , [](const auto& bank) { Assert::AreWithin(25.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("customer deposits money")
		.Given("A customer named Mr. MiddleRaster"   , [](                                      ) { return "Mr. MiddleRaster"; })
		.Given("a bank account with a $100.00 blance", [](                                      ) { return Bank{100.0}; })
		.When ("he deposits $25.00"                  , [](      auto&,                auto& bank) { bank.Deposit(25.0); })
		.When ("he deposits $25.00"                  , [](      auto&,                auto& bank) { bank.Deposit(25.0);})
		.Then ("his name is still Mr. MiddleRaster"  , [](const auto& customer, const auto&     ) { Assert::AreEqual("Mr. MiddleRaster", customer); })
		.Then ("his balance is $150.00"              , [](const auto&,          const auto& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); })
	,
	Fluent::BDD("Let's see what happens if I create 2 banks")
		.Given("I open a checking account",     [](                                         ) { return Bank{111.11}; })
		.Given("I open a savings account" ,     [](                                         ) { return Bank{222.22}; })
		.When ("I withdraw $50 from savings",   [](      auto&,                Bank& savings) { savings.Withdraw(50.0); })
		.When ("I deposit $50 into checking",   [](      auto& checking,       auto&        ) { checking.Deposit(50.0); })
		.Then ("I have $161.11 in checking",	[](const Bank& checking, const auto&        ) { Assert::AreWithin(161.11, checking.Balance(), .0001); })
		.Then ("I have $172.22 in checking",	[](const Bank&,          const Bank& savings) { Assert::AreWithin(172.22,  savings.Balance(), .0001); })
	,
	Fluent::BDD("Let's see what happens if I do oddly ordered steps")
		.When ("just checking that it doesn't blow up", [](                                ) {})
		.Given("I open a checking account"            , [](                                ) { return Bank{111.11}; })
		.When("I deposit $50 into checking"           , [](      auto& checking            ) { checking.Deposit(50.0); })
		.Then("I have $161.11 in checking"            , [](const Bank& checking            ) { Assert::AreWithin(161.11, checking.Balance(), .0001); })
		.Given("I open a savings account"             , [](                                ) { return Bank{ 222.22 }; })
		.When("I withdraw $50 from savings"           , [](      auto&,       Bank& savings) { savings.Withdraw(50.0); })
		.Then("I have $172.22 in checking"            , [](const Bank&, const Bank& savings) { Assert::AreWithin(172.22,  savings.Balance(), .0001); })
	,
};
