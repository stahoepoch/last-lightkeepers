#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Loot/LLKInteractorComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKRelicHoldTest,
	"LLK.Loot.RelicHoldRate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLLKRelicHoldTest::RunTest(const FString& Parameters)
{
	constexpr float LitSeconds = 1.5f;
	constexpr float DarkSeconds = 4.f;

	const float LitOneSecond = ULLKInteractorComponent::AdvanceRelicTakeProgress(
		0.f, 1.f, true, LitSeconds, DarkSeconds);
	const float DarkOneSecond = ULLKInteractorComponent::AdvanceRelicTakeProgress(
		0.f, 1.f, false, LitSeconds, DarkSeconds);
	TestTrue(TEXT("light makes the same held second materially faster"), LitOneSecond > DarkOneSecond * 2.f);
	TestNearlyEqual(TEXT("one lit second fills two thirds"), LitOneSecond, 1.f / LitSeconds, KINDA_SMALL_NUMBER);
	TestNearlyEqual(TEXT("one dark second fills one quarter"), DarkOneSecond, 1.f / DarkSeconds, KINDA_SMALL_NUMBER);

	const float ProgressAfterLight = ULLKInteractorComponent::AdvanceRelicTakeProgress(
		0.f, 1.f, true, LitSeconds, DarkSeconds);
	const float ProgressAfterDark = ULLKInteractorComponent::AdvanceRelicTakeProgress(
		ProgressAfterLight, 1.f, false, LitSeconds, DarkSeconds);
	TestTrue(TEXT("losing light adds slower work instead of resetting progress"),
		ProgressAfterDark > ProgressAfterLight);
	TestNearlyEqual(TEXT("one lit then one dark second is honest accumulated work"),
		ProgressAfterDark, (1.f / LitSeconds) + (1.f / DarkSeconds), KINDA_SMALL_NUMBER);

	return true;
}

#endif
