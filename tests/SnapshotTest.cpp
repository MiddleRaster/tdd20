import tdd20;
import std;
using namespace TDD20;


// can't prevent clients from writing code like this; handled by iterating over a snapshot of the test vector
void AddTests()
{
	for (int i=0; i<1000; ++i)
	{
		Test test("another test", AddTests);
	}
}
Test DontDoThis("Don't do this", AddTests);
