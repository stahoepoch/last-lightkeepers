// Roadmap task M0.7 — the first automation test.
//
// Its job is NOT to test gameplay. Nothing in this file knows what a lantern
// is. Its job is to prove the automation harness runs at all, and - the part
// that actually matters - that it can go RED. From docs/ROADMAP.md:
//
//     "It fails when deliberately broken. A green suite that cannot go red is
//      worse than no suite."
//
// A green suite that cannot fail is the most expensive thing in this project:
// it buys false confidence for every milestone after it. So the second test
// below exists purely to be broken on purpose, and the procedure for doing
// that is written out at the bottom of this file rather than left to folklore.
//
// Real gameplay assertions arrive at M1.8 (the storm charge rules, in-engine).
// The 151 data invariants in tests/balance.test.mjs keep running on plain Node
// and need no engine, so they are not duplicated here.

#include "LLK.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// EditorContext because M0.6 runs these headless through the editor
// commandlet; ProductFilter so they run in the default CI pass rather than
// only under a stress or perf filter.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKHarnessRunsTest,
	"LLK.Harness.Runs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKHarnessRunsTest::RunTest(const FString& Parameters)
{
	// If this line executes, the module compiled, the test was discovered, and
	// the runner invoked it. That is the whole of M0.7's first half.
	UE_LOG(LogLLK, Log, TEXT("LLK automation harness reached RunTest."));

	TestTrue(TEXT("the harness can run a test in the LLK module"), true);
	TestEqual(TEXT("the harness can compare values"), 2 + 2, 4);
	TestEqual(
		TEXT("the harness can compare strings, which is how every later failure will read"),
		FString(TEXT("Lanternhold")),
		FString(TEXT("Lanternhold")));

	return !HasAnyErrors();
}

// ---------------------------------------------------------------------------
// The canary. Do not delete this test, and do not make it clever.
// ---------------------------------------------------------------------------
//
// M0.7 is not done when a test passes. It is done when someone has watched one
// FAIL. This test exists so that proving it costs one character rather than an
// afternoon of writing a throwaway.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKHarnessCanFailTest,
	"LLK.Harness.CanFail",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLLKHarnessCanFailTest::RunTest(const FString& Parameters)
{
	// Flip this to false, rebuild, run the suite, and watch it go red. Then
	// flip it back. If it does NOT go red, the harness is lying and every
	// milestone after M0 is being judged by something that cannot say no.
	constexpr bool bCanaryShouldPass = true;

	TestTrue(
		TEXT("the canary is healthy - set bCanaryShouldPass to false to prove the suite can fail"),
		bCanaryShouldPass);

	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS

// ---------------------------------------------------------------------------
// How to satisfy M0.7, in order
// ---------------------------------------------------------------------------
//
//   1. Build the editor target.
//   2. Run the suite. Both tests above pass. This proves discovery and
//      execution, and it is the part that is easy to mistake for done.
//   3. Open this file, set bCanaryShouldPass to false, rebuild, run again.
//      LLK.Harness.CanFail must fail, the run must exit NON-ZERO, and CI must
//      report it. A failing test that exits zero is the same defect as no test
//      at all, wearing a green tick.
//   4. Set it back to true. Rebuild. Confirm green.
//   5. Record in docs/ROADMAP.md that step 3 was actually observed, by whom,
//      and on which date - not that it "should" work.
//
// Step 3 is the deliverable. Steps 1, 2 and 4 are setup for it.
//
// Note for whoever owns M0.6: the exit code matters as much as the output.
// `Automation RunTests LLK` has to return non-zero on failure or CI will go
// green over a red suite, which is strictly worse than having no CI.
