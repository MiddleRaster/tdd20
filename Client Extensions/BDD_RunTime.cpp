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


// a simple combination RSpec-style with Gherkin syntax BDD test harness, like BoBW but with less work done at compile-time

namespace BDD_Runtime
{
	using State = std::vector<std::any>;
	template <typename T> T& Nth(State& state, std::size_t occurrence=0)
	{
		for (auto& item : state)
			if (T* candidate = std::any_cast<T>(&item))
				if (occurrence-- == 0) return *candidate;
		throw std::logic_error("fewer values of the requested type than were requested");
	}
	
	class Step
	{
		using Ancestor = std::function<State   (      )>;
		using Setup    = std::function<std::any(State&)>;
		template <typename T>                               struct Traits : Traits<decltype(&T::operator())> {};
		template <typename C, typename R, typename... Args> struct Traits<R(C::*)(Args...) const> { using Params = std::tuple<Args...>; };
		template <typename C, typename R, typename... Args> struct Traits<R(C::*)(Args...)      > { using Params = std::tuple<Args...>; };

		const std::string name;
		const Setup setup;
		std::vector<Step> children; // not const so that it can be moved from (which happens in parent's ctor)

		static Ancestor Compose(Ancestor ancestor, Setup setup)
		{
			return [ancestor = std::move(ancestor), setup = std::move(setup)]() mutable
				{
					State state = ancestor();
					std::any added = setup(state);
					if (added.has_value()) state.push_back(std::move(added));
					return state;
				};
		}
		template <typename Callable> static Setup Adapt(Callable callable)
		{
			constexpr bool takesNothing = std::is_invocable_v<Callable&>;
			constexpr bool takesState   = std::is_invocable_v<Callable&, State&>;
			static_assert(takesNothing || takesState || std::tuple_size_v<typename Traits<Callable>::Params> == 1, "a step takes no parameter, the State, or one typed value");
			return [callable](State& state) mutable -> std::any
				{
					auto run = [&callable](auto&&... args) -> std::any
						{
							if constexpr (std::is_void_v<decltype(callable(std::forward<decltype(args)>(args)...))>) {        callable(std::forward<decltype(args)>(args)...); return {}; }
							else                                                                                     { return callable(std::forward<decltype(args)>(args)...); }
						};
					if      constexpr (takesNothing) return (void)state, run();
					else if constexpr (takesState  ) return run(state);
					else {
						if (state.size() != 1) throw std::logic_error("a single-value step requires exactly one Given value; take State& instead");
						return run(std::any_cast<std::remove_cvref_t<std::tuple_element_t<0,typename Traits<Callable>::Params>>&>(state[0]));
					}
				};
		}
	public:
		template <typename Callable, typename... Children> Step(std::string name, Callable&& callable, Children&&... child)
			: name(std::move(name))
			, setup(Adapt(std::forward<Callable>(callable)))
		{
			children.reserve(sizeof...(Children));
			(children.push_back(std::forward<Children>(child)), ...);
		}
		void Register(std::string path, Ancestor const& ancestor) const
		{
			path += " " + name;
			Ancestor composed = Compose(ancestor, setup);
			if (false == children.empty())
				for (auto const& child : children) child.Register(path, composed);
			else
				TDD20::Test(path.substr(1), [composed]() mutable { composed(); });
		}
	};
	struct Specification
	{
		Specification(Step step) { step.Register(std::string{}, []() { return State{}; }); }
	};
	template <typename Callable                      > Step Then (std::string name, Callable&& test                         ) { return Step(std::move(name), std::forward<Callable>( test)                                     ); }
	template <typename Callable, typename... Children> Step When (std::string name, Callable&& setup, Children&&... children) { return Step(std::move(name), std::forward<Callable>(setup), std::forward<Children>(children)...); }
	template <typename Callable, typename... Children> Step Given(std::string name, Callable&& setup, Children&&... children) { return Step(std::move(name), std::forward<Callable>(setup), std::forward<Children>(children)...); }
}

namespace BankSteps
{
	inline auto Deposit (double amount) { return [amount](Bank& bank) { bank.Deposit (amount); }; }
	inline auto Withdraw(double amount) { return [amount](Bank& bank) { bank.Withdraw(amount); }; }
}

using namespace BDD_Runtime;

Specification specs[] = {
	Given("[Runtime] Bank"                       , [](          ) { return Bank{}; },
		Then("starts with a zero balance"        , [](Bank& bank) { Assert::AreWithin(0.00, bank.Balance(), 0.0001); }),
		When("with sufficient funds"             , BankSteps::Deposit(100.00),
			When("withdrawal"                    , BankSteps::Withdraw(50.00),
				Then("the balance is reduced"    , [](Bank& bank) { Assert::IsTrue  (100.00 >  bank.Balance(), "balance should be less than 100.0"); }),
				Then("the balance is nonnegative", [](Bank& bank) { Assert::IsTrue  (  0.00 <= bank.Balance(), "balance cannot go negative"); }),
				Then("the balance is $50.00"     , [](Bank& bank) { Assert::AreWithin(50.00,   bank.Balance(), 0.0001); })
			)
		),
		When("account belonging to MiddleRaster"     , [](Bank &      ) { return "MiddleRaster"; }, // a second value: steps below take State&
			When("withdrawal with insufficient funds", [](State& state) { Nth<Bank>(state).Deposit(0.01); },
				Then("is the same customer"          , [](State& state) { Assert::AreEqual("MiddleRaster", Nth<const char*>(state), "wrong customer!"); }),
				Then("raises an error"               , [](State& state) { Assert::ExpectingException<std::invalid_argument>([&bank = Nth<Bank>(state)]() { bank.Withdraw(50.00); }, "cannot withdraw more than balance"); })
			)
		)
	),
	Given("[Runtime] returning void", []() {}, // a Given's lambda can return void
		Then("is ok"                , []() {})
	),
	Given("[Runtime] impossible opening of account with a negative balance", []() { Assert::ExpectingException<std::invalid_argument>([]() { return Bank{-0.001}; }, "cannot open account with negative balance"); }
	),
	Given("[Runtime] some customer's name"       , [](                     ) { return "MiddleRaster"; },
		When("is not pseudonym"                  , [](const char*& customer) { Assert::ExpectingException<TDD20::AssertException>([&customer]() { Assert::AreNotEqual("MiddleRaster", customer); }); })
	),
	Given("[Runtime] a national bank"            , [](            ) { return "Wells-Fargo"; },
		Given("and a bank account"               , [](const char*&) { return Bank{}; },
			Then("the account is empty"          , [](State& state) { Assert::AreWithin(0.00, Nth<Bank>(state).Balance(), 0.0001); }),
			Given("a customer"                   , [](State&      ) { return "Mr. MiddleRaster"; },
				When("he deposits $100.00"       , [](State& state) { Nth<Bank>(state).Deposit(100.00); },
					Then("the balance is $100.00", [](State& state) { Assert::AreWithin(100.00, Nth<Bank>(state).Balance(), 0.0001); })
				)
			)
		)
	),
	Given("[Runtime] a well-known bank"                                     , [](            ) { return "Wells Fargo"; },
		When("there's a customer"                                           , [](const char*&) { return "Mr. MiddleRaster"; },
			Given("with a savings account"                                  , [](State&      ) { return Bank{}; },
				Given("and a checking account"                              , [](State&      ) { return Bank{}; },
					When("he deposits $100 into savings"                    , [](State& state) { Nth<Bank>(state, 0).Deposit (100.00); },
						When("he transfers $50 from savings into checking"  , [](State& state) { Nth<Bank>(state, 0).Withdraw( 50.00);
																								 Nth<Bank>(state, 1).Deposit ( 50.00); },
							Then("he has $50 in savings and $50 in checking", [](State& state) { auto& savings  = Nth<Bank>(state, 0);
																								 auto& checking = Nth<Bank>(state, 1);
																								 Assert::AreWithin(50.00,  savings.Balance(), 0.0001);
																								 Assert::AreWithin(50.00, checking.Balance(), 0.0001);
																								 Assert::AreEqual("Wells Fargo"     , Nth<const char*>(state, 0));
																								 Assert::AreEqual("Mr. MiddleRaster", Nth<const char*>(state, 1));
																							   }
							)
						)
					)
				)
			)
		)
	),
};
