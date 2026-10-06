import tdd20;
import std;
using namespace TDD20;


// how to write parameterized tests

// system under test
struct Date { int year, month, day; };
Date ParseDate(const std::string& date)
{
    return {std::stoi(date.substr(0, 4)),
            std::stoi(date.substr(5, 2)),
            std::stoi(date.substr(8, 2))};
}

// parameterized tests just need to set up the lambda for the Test base class properly
struct DateTest : Test
{
    DateTest(const std::string& name, std::string text, int expectedYear, int expectedMonth, int expectedDay)
        : Test(name, [text = std::move(text), expectedYear, expectedMonth, expectedDay]()
            {
                auto date = ParseDate(text);
                Assert::AreEqual(expectedYear,  date.year);
                Assert::AreEqual(expectedMonth, date.month);
                Assert::AreEqual(expectedDay,   date.day);
            })
    {}
};
DateTest dateTests[] =
{
    {"ISO date",       "2026-10-04", 2026, 10, 4},
    {"New Year's Day", "2027-01-01", 2027,  1, 1},
    {"Leap day",       "2028-02-29", 2028,  2, 29},
};
