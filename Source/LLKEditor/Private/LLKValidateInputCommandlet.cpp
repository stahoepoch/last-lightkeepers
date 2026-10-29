#include "LLKValidateInputCommandlet.h"

#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"

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

	bool HasMappableBinding(const UInputMappingContext* Context, const UInputAction* Action, const FKey& Key)
	{
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (Mapping.Action == Action && Mapping.Key == Key && Mapping.IsPlayerMappable())
			{
				return true;
			}
		}
		return false;
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
			if (!HasMappableBinding(Context, Action, Key))
			{
				Fail(FString::Printf(TEXT("%s is missing a player-mappable keyboard binding for %s."), Spec.Label, *Key.ToString()));
				bValid = false;
			}
		}

		if (!HasMappableBinding(Context, Action, Spec.GamepadKey))
		{
			Fail(FString::Printf(TEXT("%s is missing a player-mappable gamepad binding for %s."), Spec.Label, *Spec.GamepadKey.ToString()));
			bValid = false;
		}
	}

	if (!bValid)
	{
		return 1;
	}

	UE_LOG(LogLLKInputValidation, Display, TEXT("LLK input asset contract passed: six actions, keyboard and gamepad bindings, all mappable."));
	return 0;
}
