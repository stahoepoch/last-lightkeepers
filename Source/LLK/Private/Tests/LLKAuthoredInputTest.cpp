// The authored input assets, against the code path they replace. M1.6.
//
// **The gap this closes.** Codex's editor validator checks that the seven
// actions exist, carry the right value types, and have player-mappable
// keyboard and gamepad bindings. It does not check **modifiers**, and nothing
// else did either - so `IMC_LLK` could pass every gate in the project while
// the keeper strafed down the forward axis.
//
// That failure is the worst shape available: it looks like nothing. The assets
// load, the validator is green, the keys all respond, and the character
// controller just feels wrong in a way nobody can point at. I warned Chris
// about exactly this while he was authoring them, which is a poor substitute
// for a test.
//
// **So the property asserted is agreement, not correctness in isolation.**
// `ALLKCharacter::BuildTransientBindings` is the shape movement was built and
// tuned against, and `LLK.Input.TransientFallback` already pins it. The
// authored context is about to replace it. The thing that must be true is
// that **swapping input sources does not change how the keeper moves** - so
// this builds both and compares them key by key, rather than restating the
// expected modifiers and letting the two drift apart in two files.
//
// **It skips when the assets are absent, deliberately and loudly.** A clean
// clone and the CI runner have no `/Game/Input`, and a test that failed there
// would be a test somebody disables. Absent is a legitimate state - the
// transient fallback is what it is for.

#include "Characters/LLKCharacter.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKAuthoredInputTest,
	"LLK.Input.AuthoredContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	/** A key's modifier shape, reduced to what actually changes movement. */
	struct FShape
	{
		bool bFound = false;
		bool bSwizzled = false;
		bool bNegateX = false;
		bool bNegateY = false;

		bool operator==(const FShape& Other) const
		{
			return bFound == Other.bFound
				&& bSwizzled == Other.bSwizzled
				&& bNegateX == Other.bNegateX
				&& bNegateY == Other.bNegateY;
		}

		FString Describe() const
		{
			if (!bFound)
			{
				return TEXT("not mapped");
			}
			return FString::Printf(TEXT("swizzle=%s negateX=%s negateY=%s"),
				bSwizzled ? TEXT("y") : TEXT("n"),
				bNegateX ? TEXT("y") : TEXT("n"),
				bNegateY ? TEXT("y") : TEXT("n"));
		}
	};

	/**
	 * Summarises every mapping of `Key`, across all of them.
	 *
	 * Across rather than first-found: the authored context may split a key
	 * over two rows where the transient one uses a single row with two
	 * modifiers, and those are the same behaviour. Comparing shapes rather
	 * than row structure is what lets the two be authored differently and
	 * still be checked against each other.
	 */
	FShape ShapeOf(const UInputMappingContext* Context, const UInputAction* Action, const FKey& Key)
	{
		FShape Shape;
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (Mapping.Key != Key || (Action != nullptr && Mapping.Action != Action))
			{
				continue;
			}
			Shape.bFound = true;
			for (const UInputModifier* Modifier : Mapping.Modifiers)
			{
				if (Modifier == nullptr)
				{
					continue;
				}
				if (Modifier->IsA(UInputModifierSwizzleAxis::StaticClass()))
				{
					Shape.bSwizzled = true;
				}
				else if (const UInputModifierNegate* Negate = Cast<UInputModifierNegate>(Modifier))
				{
					Shape.bNegateX |= Negate->bX;
					Shape.bNegateY |= Negate->bY;
				}
			}
		}
		return Shape;
	}
}

bool FLLKAuthoredInputTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK authored input test reached RunTest."));

	const UInputMappingContext* Authored =
		LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_LLK.IMC_LLK"));

	if (Authored == nullptr)
	{
		// Not a failure. A clean clone has no `/Game/Input`, the transient
		// fallback covers it, and a test that went red here is a test
		// somebody switches off.
		UE_LOG(LogTemp, Log,
			TEXT("LLK.Input.AuthoredContext: no /Game/Input/IMC_LLK, so there is nothing to ")
			TEXT("compare. The transient fallback is the live path and LLK.Input.TransientFallback ")
			TEXT("covers it. Not a failure."));
		return true;
	}

	// The shape movement was built and tuned against.
	const FLLKInputBindings Transient = ALLKCharacter::BuildTransientBindings(GetTransientPackage());
	if (!TestTrue(TEXT("the transient bindings are available to compare against"), Transient.IsComplete()))
	{
		return false;
	}

	// ---- the authored actions exist and are typed ----------------------
	//
	// Codex's validator checks this too. Repeated here because this test is
	// worthless if it compares against assets that are not the right ones,
	// and because a missing action would otherwise read as "not mapped" and
	// compare equal to nothing.
	struct FActionUnderTest
	{
		const TCHAR* Path;
		EInputActionValueType Type;
	};
	static const FActionUnderTest Actions[] = {
		{TEXT("/Game/Input/IA_LLK_Move.IA_LLK_Move"), EInputActionValueType::Axis2D},
		{TEXT("/Game/Input/IA_LLK_Look.IA_LLK_Look"), EInputActionValueType::Axis2D},
		{TEXT("/Game/Input/IA_LLK_Sprint.IA_LLK_Sprint"), EInputActionValueType::Boolean},
		{TEXT("/Game/Input/IA_LLK_Crouch.IA_LLK_Crouch"), EInputActionValueType::Boolean},
		{TEXT("/Game/Input/IA_LLK_Jump.IA_LLK_Jump"), EInputActionValueType::Boolean},
		{TEXT("/Game/Input/IA_LLK_Lantern.IA_LLK_Lantern"), EInputActionValueType::Boolean},
		{TEXT("/Game/Input/IA_LLK_Interact.IA_LLK_Interact"), EInputActionValueType::Boolean},
	};

	for (const FActionUnderTest& Entry : Actions)
	{
		const UInputAction* Action = LoadObject<UInputAction>(nullptr, Entry.Path);
		if (TestNotNull(*FString::Printf(TEXT("%s exists"), Entry.Path), Action))
		{
			TestEqual(*FString::Printf(TEXT("%s has the right value type"), Entry.Path),
				Action->ValueType, Entry.Type);
		}
	}

	const UInputAction* AuthoredMove =
		LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LLK_Move.IA_LLK_Move"));

	// Diagnostic, not an assertion. Modifiers can also live on the Input
	// ACTION, where they apply to every key bound to it - which for Move
	// would negate W A S and D together and is wrong in a different way. If
	// the mappings look bare, this says whether they were put here instead,
	// so the report names the actual mistake rather than guessing at it.
	if (AuthoredMove != nullptr)
	{
		UE_LOG(LogTemp, Log,
			TEXT("LLK.Input.AuthoredContext: IA_LLK_Move carries %d action-level modifier(s); ")
			TEXT("per-key modifiers belong on the IMC mappings, not here."),
			AuthoredMove->Modifiers.Num());
	}
	const UInputAction* AuthoredLook =
		LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LLK_Look.IA_LLK_Look"));
	if (AuthoredMove == nullptr || AuthoredLook == nullptr)
	{
		return false;
	}

	// ---- THE COMPARISON, which is the point of the file -----------------
	//
	// Five keys decide how the keeper moves. If the authored context differs
	// from the tuned code path on any of them, movement changes the day the
	// assets go live - and nothing else in the project would notice.
	struct FKeyUnderTest
	{
		FKey Key;
		const UInputAction* AuthoredAction;
		const UInputAction* TransientAction;
		const TCHAR* Why;
	};
	const TArray<FKeyUnderTest> Keys = {
		{EKeys::W, AuthoredMove, Transient.Move.Get(), TEXT("forward is plain X")},
		{EKeys::S, AuthoredMove, Transient.Move.Get(), TEXT("back is X negated, never swizzled")},
		{EKeys::D, AuthoredMove, Transient.Move.Get(), TEXT("right swizzles onto Y, unnegated")},
		{EKeys::A, AuthoredMove, Transient.Move.Get(), TEXT("left swizzles AND negates Y - the one that breaks")},
		{EKeys::Mouse2D, AuthoredLook, Transient.Look.Get(), TEXT("mouse pitch is inverted so forward looks up")},
	};

	for (const FKeyUnderTest& Entry : Keys)
	{
		const FShape Expected = ShapeOf(Transient.Context, Entry.TransientAction, Entry.Key);
		const FShape Actual = ShapeOf(Authored, Entry.AuthoredAction, Entry.Key);

		TestTrue(*FString::Printf(TEXT("%s is mapped in the authored context"), *Entry.Key.ToString()),
			Actual.bFound);

		TestTrue(
			*FString::Printf(TEXT("%s matches the tuned code path (%s) - authored: %s, expected: %s"),
				*Entry.Key.ToString(), Entry.Why, *Actual.Describe(), *Expected.Describe()),
			Actual == Expected);
	}

	// ---- and the shape itself, stated once ------------------------------
	//
	// Belt and braces. If both sides drifted together - someone "fixing" the
	// fallback to match a broken asset - the comparison above would pass
	// while the keeper strafed forwards. These four are the absolute rules.
	const FShape Left = ShapeOf(Authored, AuthoredMove, EKeys::A);
	const FShape Right = ShapeOf(Authored, AuthoredMove, EKeys::D);
	const FShape Back = ShapeOf(Authored, AuthoredMove, EKeys::S);

	TestTrue(TEXT("A swizzles onto the strafe axis"), Left.bSwizzled);
	TestTrue(TEXT("and A negates Y, not X - swizzle runs first"), Left.bNegateY && !Left.bNegateX);
	TestTrue(TEXT("D swizzles onto the strafe axis"), Right.bSwizzled);
	TestFalse(TEXT("and D is not negated at all, or A and D are the same key"),
		Right.bNegateX || Right.bNegateY);
	TestFalse(TEXT("S is not swizzled, or back becomes a strafe"), Back.bSwizzled);
	TestTrue(TEXT("and S negates X"), Back.bNegateX);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
