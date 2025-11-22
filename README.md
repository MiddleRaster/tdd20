# tdd20

A macro-free, module-only TDD/unit testing harness for C++20 or later.

### Sample uage:

```
import tdd20;
using namespace TDD20;

Test someFailingTests[] = 
{
  {"generic pointer test",    []{ class X{} a,b; Assert::AreEqual(&a, &b);}},// compares hex addresses
  {"just failing right off",  []{ Assert::Fail("Yep, failed"); } },
  {"an AreNotEqual test",     []{ Assert::AreNotEqual("hello", "hello"); }},
  {"doubles to 15 places",    []{ Assert::AreWithin(std::numbers::pi, 3.14, 0.0001, "not close enough"); } },
  {"an IsFalse test",         []{ Assert::IsFalse(true,  "this should be false"); } },
  {"an IsTrue test",          []{ Assert::IsTrue (false, "this should be true" ); } },
  {"asserts if no exception", []{ Assert::ExpectingException<std::exception>([]() { /* no throw */ } },
};

```

### Design goals:
1. Module-only
1. No macros
1. Minimal dependencies (only C++ standard library: ```import std;```)
1. Auto-registration of tests
1. Fast to build; fast to run
1. Easy to understand
1. Flexible and extensible

### The folder structure:
1. **main** contains the main() entry point.
1. **module** contains the single module interface file for the test harness.
1. **tests** contains some sample tests, a few of which report errors.
1. **exe** contains the project file for building the test executable.

### How it works:
A test is an instance of the ```Test``` struct that holds onto the name and a ````std::function<void()>```` representing the test body;
you'll probably want to use a lambda.
Tests are typically held in one or more arrays and are auto-registered.
Running the tests is done by calling the static
method ```static std::pair<int, int> Test::RunTests(auto&& matcher, auto&& out);```.

There are two customization points:
1. You can write your own matcher to specify which tests to run; the default matcher runs those tests whose names match the command-line arguments, if any, or else all of them.
1. You can customize the output by writing your own ```out``` class (must implement ```operator<<(const std::string&);``` e.g. std::ostream).

### How to use:

See examples in the **tests** folder for how to write tests.
The supplied ```main.cpp``` has the following command-line options:
 - ```-dump```, ```--dump```, ```-d``` or ```/dump```: dumps all test names, then exits.
 - the name of one or more tests:  e.g., ```tdd20.exe "SomeMoreTests::TestSeven" "an IsFalse test"```.
 - no arguments: runs all tests.
 - The output is exactly in the format Visual Studio expects, so if you run the .exe as a post-build step in Visual Studio, the test results will appear in the "Error List" and "Output" windows.
 - The ```Assert``` class supplies the following methods:

<div style="margin-left:40px">

| Method                   | Use  |
|:-------------------------|:-----|
| ```AreEqual```           | compares two values for equality |
| ```AreNotEqual```        | compares two values for inequality |
| ```IsTrue```             | compares a value to true |
| ```IsFalse```            | compares a value to false |
| ```Fail```               | fails with a message |
| ```IsWithin```           | compares two doubles for approximate equality within a tolerance |
| ```ExpectingException``` | expects a callable to throw an exception of a specified type |
</div>

### Running the tests:
After building the .exe project, you can add the executable as a post-build step, if you so desire.
The output will be captured by Visual Studio and shown in the "Error List" and "Output" windows, as ***warnings***.
Double-clicking on one will take you to the corresponding line in the source code.

### Visual Studio Test Explorer integration
If you *really* want to run your tests inside Visual Studio's Test Explorer, see my [Test Adapter](https://github.com/MiddleRaster/Tdd20.TestAdapter).
I don't really recommend doing so however, because Visual Studio's Test Explorer is quite slow, not really suitable for doing TDD.
The previous section describes the recommended way to run the tests.

### Future work:
Eventually, I'll write a GUI version and
a console version that outputs in color (red/green) for better readability.
