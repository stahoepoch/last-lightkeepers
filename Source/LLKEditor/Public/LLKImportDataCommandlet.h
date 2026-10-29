// Editor-only bridge from Intermediate/LLKDataImport into generated assets.
//
// The commandlet deliberately consumes the canonical Unreal payload emitted by
// Tools/import_data.py, never data/*.json.  That keeps JSON validation and
// engine asset construction as two independently checkable steps.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "LLKImportDataCommandlet.generated.h"

UCLASS()
class LLKEDITOR_API ULLKImportDataCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	ULLKImportDataCommandlet();
	virtual int32 Main(const FString& Params) override;
};
