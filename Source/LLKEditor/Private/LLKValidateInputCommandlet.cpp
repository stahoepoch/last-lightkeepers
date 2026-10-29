#include "LLKValidateInputCommandlet.h"

#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"

DEFINE_LOG_CATEGORY_STATIC(LogLLKInputValidation, Log, All);

namespace LLKInputValidation
{
	struct FActionSpec
	{
		const TCHAR* Label;
		const TCHAR* ObjectPath;
		EInputActionValueType ValueType;
		TArray<FKey> KeyboardKeys;
		FKey GamepadKey;
	};

	bool Fail(const FString& Message)
	{
		UE_LOG(LogLLKInputValidation, Error, TEXT("%s"), *Message);
		return false;
	}

	enum class EBindingStatus : uint8
	{
		Missing,
		NotPlayerMappable,
		Valid,
	};

	const FEnhancedActionKeyMapping* FindBinding(const UInputMappingContext* Context, const UInputAction* Action, const FKey& Key)
	{
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (Mapping.Action == Action && Mapping.Key == Key)
			{
				return &Mapping;
			}
		}
		return nullptr;
	}

	EBindingStatus GetBindingStatus(const UInputMappingContext* Context, const UInputAction* Action, const FKey& Key)
	{
		const FEnhancedActionKeyMapping* Mapping = FindBinding(Context, Action, Key);
		if (Mapping == nullptr)
		{
			return EBindingStatus::Missing;
		}

		return Mapping->IsPlayerMappable() ? EBindingStatus::Valid : EBindingStatus::NotPlayerMappable;
	}

	bool ValidateBinding(const UInputMappingContext* Context, const UInputAction* Action, const FActionSpec& Spec,
		const FKey& Key, const TCHAR* Device)
	{
		switch (GetBindingStatus(Context, Action, Key))
		{
		case EBindingStatus::Valid:
			return true;
		case EBindingStatus::NotPlayerMappable:
			Fail(FString::Printf(
				TEXT("%s has a %s binding for %s, but it is not player-mappable. Enable Player Mappable Key Settings on that mapping."),
				Spec.Label, Device, *Key.ToString()));
			return false;
		case EBindingStatus::Missing:
			Fail(FString::Printf(TEXT("%s is missing a %s binding for %s."), Spec.Label, Device, *Key.ToString()));
			return false;
		}

		return false;
	}

	bool HasNegate(const FEnhancedActionKeyMapping& Mapping, bool bX, bool bY)
	{
		for (const UInputModifier* Modifier : Mapping.Modifiers)
		{
			if (const UInputModifierNegate* Negate = Cast<UInputModifierNegate>(Modifier);
				Negate != nullptr && Negate->bX == bX && Negate->bY == bY && !Negate->bZ)
			{
				return true;
			}
		}
		return false;
	}

	bool HasSwizzle(const FEnhancedActionKeyMapping& Mapping)
	{
		for (const UInputModifier* Modifier : Mapping.Modifiers)
		{
			if (const UInputModifierSwizzleAxis* Swizzle = Cast<UInputModifierSwizzleAxis>(Modifier);
				Swizzle != nullptr && Swizzle->Order == EInputAxisSwizzle::YXZ)
			{
				return true;
			}
		}
		return false;
	}

	bool HasSwizzleThenNegateY(const FEnhancedActionKeyMapping& Mapping)
	{
		int32 SwizzleIndex = INDEX_NONE;
		int32 NegateYIndex = INDEX_NONE;
		for (int32 Index = 0; Index < Mapping.Modifiers.Num(); ++Index)
		{
			const UInputModifier* Modifier = Mapping.Modifiers[Index];
			if (const UInputModifierSwizzleAxis* Swizzle = Cast<UInputModifierSwizzleAxis>(Modifier);
				Swizzle != nullptr && Swizzle->Order == EInputAxisSwizzle::YXZ)
			{
				SwizzleIndex = Index;
			}
			if (const UInputModifierNegate* Negate = Cast<UInputModifierNegate>(Modifier);
				Negate != nullptr && !Negate->bX && Negate->bY && !Negate->bZ)
			{
				NegateYIndex = Index;
			}
		}

		return SwizzleIndex != INDEX_NONE && NegateYIndex != INDEX_NONE && SwizzleIndex < NegateYIndex;
	}

	void ValidateMovementModifiers(const UInputMappingContext* Context, bool& bValid)
	{
		const UInputAction* Move = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LLK_Move.IA_LLK_Move"));
		const UInputAction* Look = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/IA_LLK_Look.IA_LLK_Look"));
		if (Move == nullptr || Look == nullptr)
		{
			return;
		}

		if (const FEnhancedActionKeyMapping* Back = FindBinding(Context, Move, EKeys::S); Back != nullptr && !HasNegate(*Back, true, false))
		{
			Fail(TEXT("Move S needs a Negate modifier with only X enabled."));
			bValid = false;
		}
		if (const FEnhancedActionKeyMapping* Right = FindBinding(Context, Move, EKeys::D); Right != nullptr && !HasSwizzle(*Right))
		{
			Fail(TEXT("Move D needs a Swizzle Input Axis Values modifier using YXZ."));
			bValid = false;
		}
		if (const FEnhancedActionKeyMapping* Left = FindBinding(Context, Move, EKeys::A); Left != nullptr && !HasSwizzleThenNegateY(*Left))
		{
			Fail(TEXT("Move A needs Swizzle YXZ followed by Negate with only Y enabled."));
			bValid = false;
		}
		if (const FEnhancedActionKeyMapping* Mouse = FindBinding(Context, Look, EKeys::Mouse2D); Mouse != nullptr && !HasNegate(*Mouse, false, true))
		{
			Fail(TEXT("Look Mouse XY needs a Negate modifier with only Y enabled."));
			bValid = false;
		}
	}

	const TArray<FActionSpec>& Specs()
	{
		static const TArray<FActionSpec> Required =
		{
			{ TEXT("Move"), TEXT("/Game/Input/IA_LLK_Move.IA_LLK_Move"), EInputActionValueType::Axis2D,
				{ EKeys::W, EKeys::A, EKeys::S, EKeys::D }, EKeys::Gamepad_Left2D },
			{ TEXT("Look"), TEXT("/Game/Input/IA_LLK_Look.IA_LLK_Look"), EInputActionValueType::Axis2D,
				{ EKeys::Mouse2D }, EKeys::Gamepad_Right2D },
			{ TEXT("Sprint"), TEXT("/Game/Input/IA_LLK_Sprint.IA_LLK_Sprint"), EInputActionValueType::Boolean,
				{ EKeys::LeftShift }, EKeys::Gamepad_LeftThumbstick },
			{ TEXT("Crouch"), TEXT("/Game/Input/IA_LLK_Crouch.IA_LLK_Crouch"), EInputActionValueType::Boolean,
				{ EKeys::LeftControl }, EKeys::Gamepad_RightThumbstick },
			{ TEXT("Jump"), TEXT("/Game/Input/IA_LLK_Jump.IA_LLK_Jump"), EInputActionValueType::Boolean,
				{ EKeys::SpaceBar }, EKeys::Gamepad_FaceButton_Bottom },
			{ TEXT("Lantern"), TEXT("/Game/Input/IA_LLK_Lantern.IA_LLK_Lantern"), EInputActionValueType::Boolean,
				{ EKeys::F }, EKeys::Gamepad_DPad_Right },
			// E is the established keyboard verb for using the object in front of
			// the keeper. Keep Jump on Bottom (A / Cross) and put Interact on Left
			// (X / Square), so the two actions remain distinct on a controller.
			{ TEXT("Interact"), TEXT("/Game/Input/IA_LLK_Interact.IA_LLK_Interact"), EInputActionValueType::Boolean,
				{ EKeys::E }, EKeys::Gamepad_FaceButton_Left },
		};
		return Required;
	}
}

ULLKValidateInputCommandlet::ULLKValidateInputCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 ULLKValidateInputCommandlet::Main(const FString& Params)
{
	using namespace LLKInputValidation;

	UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Input/IMC_LLK.IMC_LLK"));
	if (Context == nullptr)
	{
		return Fail(TEXT("Missing /Game/Input/IMC_LLK. M1.6 needs the authored mapping context.")) ? 0 : 1;
	}

	bool bValid = true;
	for (const FActionSpec& Spec : Specs())
	{
		UInputAction* Action = LoadObject<UInputAction>(nullptr, Spec.ObjectPath);
		if (Action == nullptr)
		{
			Fail(FString::Printf(TEXT("Missing Input Action for %s: %s"), Spec.Label, Spec.ObjectPath));
			bValid = false;
			continue;
		}

		if (Action->ValueType != Spec.ValueType)
		{
			Fail(FString::Printf(TEXT("%s Input Action has the wrong value type."), Spec.Label));
			bValid = false;
		}

		for (const FKey& Key : Spec.KeyboardKeys)
		{
			if (!ValidateBinding(Context, Action, Spec, Key, TEXT("keyboard")))
			{
				bValid = false;
			}
		}

		if (!ValidateBinding(Context, Action, Spec, Spec.GamepadKey, TEXT("gamepad")))
		{
			bValid = false;
		}
	}

	ValidateMovementModifiers(Context, bValid);

	if (!bValid)
	{
		return 1;
	}

	UE_LOG(LogLLKInputValidation, Display, TEXT("LLK input asset contract passed: seven actions, keyboard and gamepad bindings, all mappable."));
	return 0;
}
