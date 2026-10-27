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
struct Checking : Bank {};
struct Savings : Bank {};


// how to write BDD tests

template <typename GivenTuple, typename WhenTuple, typename ThenTuple> struct BDDTest
{
	std::string name;
	GivenTuple givens;
	 WhenTuple whens;
	 ThenTuple thens;

	template <typename NewGiven> auto Given(const std::string&, NewGiven newGiven) const { return Given(newGiven); }
	template <typename NewGiven> auto Given(                    NewGiven newGiven) const { auto newGivens = std::tuple_cat(givens, std::tuple{std::move(newGiven)}); return BDDTest<decltype(newGivens), WhenTuple, ThenTuple>{name, std::move(newGivens), whens, thens}; }
	template <typename NewWhen > auto When (const std::string&, NewWhen  newWhen ) const { return When(newWhen); }
	template <typename NewWhen > auto When (                    NewWhen  newWhen ) const { auto newWhens = std::tuple_cat(whens, std::tuple{std::move(newWhen)}); return BDDTest<GivenTuple, decltype(newWhens), ThenTuple>{name, givens, std::move(newWhens), thens}; }
	template <typename NewThen > auto Then (const std::string&, NewThen  newThen ) const { return Then(newThen); }
	template <typename NewThen > auto Then (                    NewThen  newThen ) const { auto newThens = std::tuple_cat(thens, std::tuple{std::move(newThen)}); return BDDTest<GivenTuple, WhenTuple, decltype(newThens)>{name, givens, whens, std::move(newThens)}; }
};
struct Fluent
{
	inline static auto BDD(std::string name) { return BDDTest<std::tuple<>, std::tuple<>, std::tuple<>>{std::move(name),{},{},{}}; }
};

struct BDDTestSuite : public Test
{
	template<typename> struct FunctionTraits;
	template<typename C, typename R, typename... Args> struct FunctionTraits<R(C::*)(Args...) const> { using Params = std::tuple<Args...>; };
	template<typename T> using Bare = std::remove_cvref_t<T>;

	template<typename Wanted, typename Tuple, typename Seq = std::make_index_sequence<std::tuple_size_v<Tuple>>> struct FindUnique;
	template<typename Wanted, typename... Types, size_t... Is> struct FindUnique<Wanted, std::tuple<Types...>, std::index_sequence<Is...>>
	{
		static constexpr size_t Count = (size_t(0) + ... + size_t(std::same_as<Bare<Wanted>, Bare<Types>>));
		static_assert(Count != 0, "no Given produces the type this lambda asks for");
		static_assert(Count <= 1, "more than one Given produces this type; wrap them in distinct types");
		static constexpr size_t value = (size_t(0) + ... + (std::same_as<Bare<Wanted>, Bare<Types>> ? Is : size_t(0)));
	};

	template<typename Params, typename Tuple> struct MatchParameters;
	template<typename... Params, typename Tuple> struct MatchParameters<std::tuple<Params...>, Tuple> { using type = std::index_sequence<FindUnique<Params, Tuple>::value...>; };
	template<size_t... Is,typename Lambda,typename TupleT> static decltype(auto) InvokeSelected(std::index_sequence<Is...>, Lambda&& lambda, TupleT&& tuple)
	{
		return std::invoke(std::forward<Lambda>(lambda), std::get<Is>(std::forward<TupleT>(tuple))...);
	}

	template <typename GivenTuple, typename WhenTuple, typename ThenTuple> BDDTestSuite(BDDTest<GivenTuple, WhenTuple, ThenTuple> def)
		: Test(def.name, [givens = std::move(def.givens), whens = std::move(def.whens), thens = std::move(def.thens)]()
			{
				auto tuple = std::apply([ ](auto const&... given){ return std::tuple{given() ...}; }, givens);

				std::apply( [&](auto const&... when)
							{
								(
									[&]
									{
										using When = std::remove_cvref_t<decltype(when)>;
										using Params = typename FunctionTraits<decltype(&When::operator())>::Params;
										using Matches =typename MatchParameters<Params,decltype(tuple)>::type;
										InvokeSelected(Matches{},when,tuple);
									}(), ...
								);
							}, whens);

				std::apply( [&](auto const&... then)
							{
								(
									[&]
									{
										using Then =std::remove_cvref_t<decltype(then)>;
										using Params = typename FunctionTraits<decltype(&Then::operator())>::Params;
										using Matches = typename MatchParameters<Params, decltype(tuple)>::type;

										InvokeSelected(Matches{}, then,tuple);
									}(), ...
								);
							}, thens);
			})
	{}
} bddTests[] =
{
	BDDTest{
		.name   = "can open account with positive funds",
		.givens = std::tuple{[](                ) { return Bank{0.01}; } },
		.whens  = std::tuple{[](      Bank&     ) { } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(.01, bank.Balance(), .0001); } }
	},
	BDDTest{
		.name   = "cannot open account with negative funds",
		.givens = std::tuple{[](   ) { Assert::ExpectingException<std::invalid_argument>([](){ return Bank{-0.01}; }, "cannot open account with negative balance"); return 0; } },
		.whens  = std::tuple{[](int) { } },
		.thens  = std::tuple{[](int) { } }
	},
	BDDTest {
		.name   = "depositing money increases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      Bank& bank) { bank.Deposit(50.0); } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "deposit cannot be negative",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      Bank& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Deposit(-0.01); }, "cannot deposit negative amount"); } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "withdrawing money decreases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      Bank& bank) { bank.Withdraw(50.0); } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "cannot withdraw more than balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      Bank& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(100.01); }, "cannot withdraw more than balance"); } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "cannot withdraw negative amount",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
		.whens  = std::tuple{[](      Bank& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(-0.01); }, "cannot withdraw negative amount"); } },
		.thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(100.0, bank.Balance(), .0001); } }
	},
	BDDTest {
		.name   = "withdrawing money twice decreases balance",
		.givens = std::tuple{[](                ) { return Bank{100.0}; } },
	    .whens  = std::tuple{[](      Bank& bank) { bank.Withdraw(25.0); },
		                     [](      Bank& bank) { bank.Withdraw(25.0); } },
	    .thens  = std::tuple{[](const Bank& bank) { Assert::AreWithin(50.0, bank.Balance(), .0001); } }
	},

	Fluent::BDD("as a bank customer, I can open an account and deposit money into it")
		.Given("an open bank account containing $100.00", [](                ) { return Bank{100.0}; })
		.When ("I deposit $50.00"                       , [](      Bank& bank) { bank.Deposit(50.0); })
		.Then ("the balance should be $150.00"          , [](const Bank& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }),

	Fluent::BDD("as a bank customer, I can open an account and withdraw some of the funds")
		.Given("an open account"      , [](                ) { return Bank{100.0}; })
		.When ("I withdraw some funds", [](      Bank& bank) { bank.Withdraw(75.0);})
		.Then ("I have $25 remaining" , [](const Bank& bank) { Assert::AreWithin(25.0, bank.Balance(), .0001); }),

	Fluent::BDD("customer deposits money")
		.Given([](                                      ) { return std::string("Bill"); })
		.Given([](                                      ) { return Bank{100.0}; })
		.When ([](      std::string&,                Bank& bank) { bank.Deposit(25.0);})
		.When ([](      std::string&,                Bank& bank) { bank.Deposit(25.0);})
		.Then ([](const std::string& customer, const Bank&     ) { Assert::AreEqual("Bill", customer); })
		.Then ([](const std::string&,          const Bank& bank) { Assert::AreWithin(150.0, bank.Balance(), .0001); }),

	Fluent::BDD("Let's see what happens if I create 2 banks")
		.Given("I open a checking account",     [](                        ) { return Checking{111.11}; })
		.Given("I open a savings account" ,     [](                        ) { return Savings {222.22}; })
		.When ("I withdraw $50 from savings",   [](      Savings & savings ) { savings.Withdraw(50.0); })
		.When ("I deposit $50 into checking",   [](      Checking& checking) { checking.Deposit(50.0); })
		.Then ("I have $161.11 in checking",	[](const Checking& checking) { Assert::AreWithin(161.11, checking.Balance(), .0001); })
		.Then ("I have $172.22 in checking",	[](const Savings & savings ) { Assert::AreWithin(172.22,  savings.Balance(), .0001); })
	,



};
