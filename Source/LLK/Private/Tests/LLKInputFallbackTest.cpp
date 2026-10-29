// The transient input fallback, asserted.
//
// `ALLKCharacter::BuildTransientBindings` exists so the keeper can be walked
// before anyone authors an Input Action asset, because `Content/` is Chris's
// alone (row 41). It is a stopgap and it is meant to be deleted when M1.6's
// real assets land - which is exactly why it needs a test. Temporary code
// that nobody checks is how a stopgap becomes permanent and wrong.
//
// It needs no world, no pawn and no running game: the function is static and
// takes its outer, so the table can be built and read in isolation.
//
// The strafe mapping is the one that earns this file. A missing swizzle on A
// or D sends strafing down the forward axis, and a missing negate sends it
// the wrong way - both of which read as "the character controller feels
// wrong" rather than as a mapping bug, and neither of which any compiler or
// data check can see.

#include "Characters/LLKCharacter.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLLKInputFallbackTest,
	"LLK.Input.TransientFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace
{
	/** The mappings for one key, so a test can ask what modifiers it carries. */
	const FEnhancedActionKeyMapping* FindMapping(const UInputMappingContext* Context, const FKey& Key)
	{
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (Mapping.Key == Key)
			{
				return &Mapping;
			}
		}
		return nullptr;
	}

	bool HasModifier(const FEnhancedActionKeyMapping& Mapping, const UClass* ModifierClass)
	{
		for (const UInputModifier* Modifier : Mapping.Modifiers)
		{
			if (Modifier != nullptr && Modifier->IsA(ModifierClass))
			{
				return true;
			}
		}
		return false;
	}

	const UInputModifierNegate* FindNegate(const FEnhancedActionKeyMapping& Mapping)
	{
		for (const UInputModifier* Modifier : Mapping.Modifiers)
		{
			if (const UInputModifierNegate* Negate = Cast<UInputModifierNegate>(Modifier))
			{
				return Negate;
			}
		}
		return nullptr;
	}
}

bool FLLKInputFallbackTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("LLK input fallback test reached RunTest."));

	const FLLKInputBindings Bindings = ALLKCharacter::BuildTransientBindings(GetTransientPackage());

	// 1. All seven or none. A half-bound keeper reads as a physics bug.
	TestTrue(TEXT("the transient bindings are complete"), Bindings.IsComplete());
	if (!Bindings.IsComplete())
	{
		return false;
	}

	// 2. Seven distinct actions. Reusing one action for two jobs would make
	//    sprint and crouch fire each other.
	TSet<const UInputAction*> Distinct{
		Bindings.Move.Get(), Bindings.Look.Get(), Bindings.Sprint.Get(), Bindings.CrouchToggle.Get(),
		Bindings.Jump.Get(), Bindings.LanternToggle.Get(), Bindings.Interact.Get()};
	TestEqual(TEXT("the seven actions are distinct objects"), Distinct.Num(), 7);

	// 3. Value types. An Axis2D read off a Boolean action is silently zero.
	TestEqual(TEXT("Move is Axis2D"), Bindings.Move->ValueType, EInputActionValueType::Axis2D);
	TestEqual(TEXT("Look is Axis2D"), Bindings.Look->ValueType, EInputActionValueType::Axis2D);
	TestEqual(TEXT("Sprint is Boolean"), Bindings.Sprint->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Crouch is Boolean"), Bindings.CrouchToggle->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Jump is Boolean"), Bindings.Jump->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Lantern is Boolean"), Bindings.LanternToggle->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("Interact is Boolean"), Bindings.Interact->ValueType, EInputActionValueType::Boolean);

	// 4. Ten mappings: four movement, one look, sprint, crouch, jump, lantern,
	//    interact. This count caught the seventh action being added, which is
	//    the point of asserting it rather than trusting it.
	TestEqual(TEXT("the context has ten key mappings"), Bindings.Context->GetMappings().Num(), 10);

	// 5. Every key the player will press is actually present.
	for (const FKey& Key : {EKeys::W, EKeys::A, EKeys::S, EKeys::D, EKeys::Mouse2D, EKeys::LeftShift,
			EKeys::LeftControl, EKeys::SpaceBar, EKeys::F, EKeys::E})
	{
		TestNotNull(*FString::Printf(TEXT("%s is mapped"), *Key.ToString()), FindMapping(Bindings.Context, Key));
	}

	// 6. The strafe shape, which is the reason this file exists.
	//    W is forward with no modifier. S is the same axis, negated. A and D
	//    swizzle into Y, and only A is negated.
	const FEnhancedActionKeyMapping* Forward = FindMapping(Bindings.Context, EKeys::W);
	const FEnhancedActionKeyMapping* Back = FindMapping(Bindings.Context, EKeys::S);
	const FEnhancedActionKeyMapping* Right = FindMapping(Bindings.Context, EKeys::D);
	const FEnhancedActionKeyMapping* Left = FindMapping(Bindings.Context, EKeys::A);

	TestEqual(TEXT("W carries no modifier"), Forward->Modifiers.Num(), 0);
	TestTrue(TEXT("S is negated"), HasModifier(*Back, UInputModifierNegate::StaticClass()));
	TestFalse(TEXT("S is not swizzled"), HasModifier(*Back, UInputModifierSwizzleAxis::StaticClass()));

	TestTrue(TEXT("D swizzles onto the right axis"), HasModifier(*Right, UInputModifierSwizzleAxis::StaticClass()));
	TestFalse(TEXT("D is not negated"), HasModifier(*Right, UInputModifierNegate::StaticClass()));

	TestTrue(TEXT("A swizzles onto the right axis"), HasModifier(*Left, UInputModifierSwizzleAxis::StaticClass()));
	TestTrue(TEXT("A is negated"), HasModifier(*Left, UInputModifierNegate::StaticClass()));
	const UInputModifierNegate* LeftNegate = FindNegate(*Left);
	TestNotNull(TEXT("A has a concrete negate modifier"), LeftNegate);
	if (LeftNegate != nullptr)
	{
		TestFalse(TEXT("A does not negate the pre-swizzle X axis"), LeftNegate->bX);
		TestTrue(TEXT("A negates the swizzled Y axis"), LeftNegate->bY);
	}

	// 7. Mouse pitch is inverted so that pushing forward looks up.
	const FEnhancedActionKeyMapping* Look = FindMapping(Bindings.Context, EKeys::Mouse2D);
	TestTrue(TEXT("mouse look negates pitch"), HasModifier(*Look, UInputModifierNegate::StaticClass()));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
