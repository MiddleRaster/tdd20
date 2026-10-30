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
	struct Up {};
	template <typename R, typename... A> struct CallSignature
	{
		using Result = R;
		using Args = std::tuple<A...>;
	};
	template <typename T>	                         struct CallTraits                               : CallTraits<decltype(&T::operator())> {};
	template <typename R,             typename... A> struct CallTraits<R(   *)(A...)               > : CallSignature<R, A...> {};
	template <typename R,             typename... A> struct CallTraits<R(   *)(A...) noexcept      > : CallSignature<R, A...> {};
	template <typename R, typename C, typename... A> struct CallTraits<R(C::*)(A...)               > : CallSignature<R, A...> {};
	template <typename R, typename C, typename... A> struct CallTraits<R(C::*)(A...) const         > : CallSignature<R, A...> {};
	template <typename R, typename C, typename... A> struct CallTraits<R(C::*)(A...)       noexcept> : CallSignature<R, A...> {};
	template <typename R, typename C, typename... A> struct CallTraits<R(C::*)(A...) const noexcept> : CallSignature<R, A...> {};

	class LambdaList
	{
		struct Element
		{
			std::string prefix;
			using Task = std::function<std::any(std::vector<std::any>&)>;
			Task task;
		};
		std::vector<Element> tasks;
		bool test = false;

		template <typename P> static P ArgumentAt(std::vector<std::any>& results, std::size_t position)
		{	// give test authors a clue as to why their test isn't working right
			using T = std::remove_cvref_t<P>;
			T* object = std::any_cast<T>(&results[position]);
			if (object == nullptr)
				throw std::runtime_error(std::string("RSpec: parameter ") + std::to_string(position + 1) + " wants " + typeid(T).name() + ", but the ancestor value in that position is " + results[position].type().name());
			return static_cast<P>(*object);
		}
		template <typename Fn, typename... As> static std::any Call(Fn& fn, As&&... args)
		{
			if constexpr (std::is_void_v<std::invoke_result_t<Fn&, As...>>) {
				fn(std::forward<As>(args)...);
				return {};
			} else
				return fn(std::forward<As>(args)...);
		}
		template <typename Fn, typename Args> static std::any Run(Fn& fn, bool asIt, std::vector<std::any>& results)
		{
			constexpr std::size_t count = std::tuple_size_v<Args>;
			if (results.size() < count)
				throw std::runtime_error("RSpec: the lambda declares " + std::to_string(count) + " parameters, but its ancestors produced only " + std::to_string(results.size()) + " values");

			return[&]<std::size_t... Is>(std::index_sequence<Is...>) -> std::any
			{
				if (asIt)
				{
					if constexpr (std::is_invocable_v<Fn&, const std::remove_cvref_t<std::tuple_element_t<Is, Args>>&...>)
						return Call(fn, ArgumentAt<const std::remove_cvref_t<std::tuple_element_t<Is, Args>>&>(results, Is)...);
					else
						throw std::logic_error("RSpec: an It receives its ancestors' objects as const");
				}
				return Call(fn, ArgumentAt<std::tuple_element_t<Is, Args>>(results, Is)...);
			}(std::make_index_sequence<count>{});
		}

		template <typename F> requires (std::is_function_v<std::remove_pointer_t<std::decay_t<F>>> || requires { &std::decay_t<F>::operator(); })
		void Add(F&& f)
		{
			using Fn   = std::decay_t<F>;
			using Args = typename CallTraits<Fn>::Args;
			tasks.back().task = [fn = Fn(std::forward<F>(f)), asIt = test](std::vector<std::any>& results) mutable -> std::any
									{
										return Run<Fn,Args>(fn, asIt, results);
									};
		}

	public:
		LambdaList(const std::string& name, bool test = false) : test(test)
		{
			tasks.push_back({ name, [](std::vector<std::any>&) -> std::any { return {}; } });
		}

		LambdaList(const LambdaList& ) = default;
		LambdaList(      LambdaList&&) = default;
		LambdaList & operator=(const LambdaList& ) = delete;
		LambdaList&& operator=(      LambdaList&&) = delete;

		template <typename F> LambdaList operator=(F&& f)
		{
			Add(std::forward<F>(f));
			return *this;
		}

		LambdaList operator,(const LambdaList& rhs) const
		{
			LambdaList result = *this;
			result.tasks.insert(result.tasks.end(), rhs.tasks.begin(), rhs.tasks.end());

			if (rhs.test)
			{
				std::string testName;
				for (Element element : result.tasks)
					testName += " " + element.prefix;

				TDD20::Test(testName.substr(1), [tasks = result.tasks]()
					{
						std::vector<std::any> results;
						results.reserve(tasks.size());

						for (Element element : tasks)
						{
							std::any result = element.task(results);
							if (result.has_value()) // skip void returning lambdas
								results.push_back(std::move(result));
						}
					});
				result.tasks.pop_back(); // the test is registered. Remove the last "It" so we're ready for siblings
			}
			return result;
		}
		LambdaList operator,(const Up&) const
		{
			if (tasks.empty())
				throw std::logic_error("Up() climbs above the outermost Describe");

			LambdaList result = *this;
			result.tasks.pop_back();
			return result;
		}
	};

	inline LambdaList Describe(const std::string& name) { return LambdaList{name}; }
	inline LambdaList Context (const std::string& text) { return Describe(text); }
	inline LambdaList It      (const std::string& text) { return LambdaList(text, true); }
}

namespace BankSteps
{
	inline auto Deposit     (double amount) { return [amount](      Bank& bank) { bank.Deposit (amount); }; }
	inline auto Withdraw    (double amount) { return [amount](      Bank& bank) { bank.Withdraw(amount); }; }
	inline auto BalanceBelow(double  limit) { return [limit ](const Bank& bank) { Assert::IsTrue(limit > bank.Balance(), "balance should be below the limit"); }; }
}

using namespace RSpec;

auto Spec = (
Describe("Bank") = []() { return Bank{}; }
	, Describe("with sufficient funds")        = BankSteps::Deposit(100.00)
		, Context("withdrawal")                = BankSteps::Withdraw(50.00)
			, It("reduces the balance")        = [](const Bank& bank) { Assert::IsTrue(100.00 > bank.Balance(), "balance should be less than 100.0"); }
			, It("the balance is nonnegative") = [](const Bank& bank) { Assert::IsTrue( 0.00 <= bank.Balance(), "balance cannot go negative"); }
			, It("the balance is $50.00")      = [](const Bank& bank) { Assert::AreWithin(50.00, bank.Balance(), 0.0001); }
			, Up()
		, Up()
	, Describe("with insufficient funds")      = [](      Bank& bank) { bank.Deposit(0.01); }
		, Context("withdrawal")                = [](      Bank& bank) { Assert::ExpectingException<std::invalid_argument>([&bank]() { return bank.Withdraw(50.00); }, "cannot withdraw more than balance"); }
			, It("raises an error")            = [](const Bank&     ) { /* already threw, above */ }
);
