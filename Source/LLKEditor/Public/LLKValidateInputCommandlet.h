// Editor-only validation for M1.6's authored Enhanced Input assets.
//
// Chris owns Content. This commandlet never creates, renames, or saves an
// asset; it only makes the handoff concrete once those assets exist.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "LLKValidateInputCommandlet.generated.h"

UCLASS()
class LLKEDITOR_API ULLKValidateInputCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	ULLKValidateInputCommandlet();
	virtual int32 Main(const FString& Params) override;
};
