#include "LLKImportDataCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Data/LLKEnemyDef.h"
#include "Data/LLKLanternData.h"
#include "Data/LLKLootRulesData.h"
#include "Data/LLKStormChargeData.h"
#include "Data/LLKTableRows.h"
#include "Data/LLKWorkshopRulesData.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogLLKDataImport, Log, All);

namespace LLKDataImport
{
	constexpr TCHAR AssetRoot[] = TEXT("/Game/Data");

	bool Fail(const FString& Message)
	{
		UE_LOG(LogLLKDataImport, Error, TEXT("%s"), *Message);
		return false;
	}

	bool LoadObject(const FString& Filename, TSharedPtr<FJsonObject>& OutObject)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Filename))
		{
			return Fail(FString::Printf(TEXT("Could not read payload: %s"), *Filename));
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
		if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
		{
			return Fail(FString::Printf(TEXT("Invalid JSON payload %s: %s"), *Filename, *Reader->GetErrorMessage()));
		}
		return true;
	}

	bool GetObject(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, TSharedPtr<FJsonObject>& OutObject)
	{
		const TSharedPtr<FJsonObject>* FoundObject = nullptr;
		if (!Parent->TryGetObjectField(Field, FoundObject) || FoundObject == nullptr || !FoundObject->IsValid())
		{
			return Fail(FString::Printf(TEXT("Missing object field '%s'."), Field));
		}
		OutObject = *FoundObject;
		return true;
	}

	bool GetArray(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, const TArray<TSharedPtr<FJsonValue>>*& OutArray)
	{
		if (!Parent->TryGetArrayField(Field, OutArray) || OutArray == nullptr)
		{
			return Fail(FString::Printf(TEXT("Missing array field '%s'."), Field));
		}
		return true;
	}

	bool GetNumber(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, float& OutValue)
	{
		double Value = 0.0;
		if (!Parent->TryGetNumberField(Field, Value))
		{
			return Fail(FString::Printf(TEXT("Missing number field '%s'."), Field));
		}
		OutValue = static_cast<float>(Value);
		return true;
	}

	bool GetInt(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, int32& OutValue)
	{
		double Value = 0.0;
		if (!Parent->TryGetNumberField(Field, Value))
		{
			return Fail(FString::Printf(TEXT("Missing integer field '%s'."), Field));
		}
		OutValue = static_cast<int32>(Value);
		return true;
	}

	bool GetBool(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, bool& OutValue)
	{
		if (!Parent->TryGetBoolField(Field, OutValue))
		{
			return Fail(FString::Printf(TEXT("Missing boolean field '%s'."), Field));
		}
		return true;
	}

	bool GetString(const TSharedPtr<FJsonObject>& Parent, const TCHAR* Field, FString& OutValue)
	{
		if (!Parent->TryGetStringField(Field, OutValue))
		{
			return Fail(FString::Printf(TEXT("Missing string field '%s'."), Field));
		}
		return true;
	}

	TSharedRef<FJsonObject> CopyObject(const TSharedPtr<FJsonObject>& Source)
	{
		const TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->Values = Source->Values;
		return Result;
	}

	bool WriteJsonArray(const TArray<TSharedPtr<FJsonValue>>& Rows, FString& OutJson)
	{
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
		return FJsonSerializer::Serialize(Rows, Writer);
	}

	bool BuildObjectRows(const TSharedPtr<FJsonObject>& Source, TArray<TSharedPtr<FJsonValue>>& OutRows, const bool bEnemyRows = false)
	{
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Source->Values)
		{
			const TSharedPtr<FJsonObject> SourceRow = Pair.Value->AsObject();
			if (!SourceRow.IsValid())
			{
				return Fail(FString::Printf(TEXT("Table row '%s' is not an object."), *Pair.Key));
			}
			const TSharedRef<FJsonObject> Row = CopyObject(SourceRow);
			Row->SetStringField(TEXT("Name"), Pair.Key);
			if (bEnemyRows)
			{
				Row->SetStringField(TEXT("family"), Pair.Key);
			}
			OutRows.Add(MakeShared<FJsonValueObject>(Row));
		}
		return true;
	}

	bool BuildIdRows(const TArray<TSharedPtr<FJsonValue>>& Source, TArray<TSharedPtr<FJsonValue>>& OutRows)
	{
		for (const TSharedPtr<FJsonValue>& Value : Source)
		{
			const TSharedPtr<FJsonObject> SourceRow = Value->AsObject();
			if (!SourceRow.IsValid())
			{
				return Fail(TEXT("Table array contains a non-object row."));
			}
			FString Id;
			if (!SourceRow->TryGetStringField(TEXT("id"), Id) || Id.IsEmpty())
			{
				return Fail(TEXT("Table array row is missing its id."));
			}
			const TSharedRef<FJsonObject> Row = CopyObject(SourceRow);
			Row->Values.Remove(TEXT("id"));
			Row->SetStringField(TEXT("Name"), Id);
			OutRows.Add(MakeShared<FJsonValueObject>(Row));
		}
		return true;
	}

	void NormaliseLootTierRows(TArray<TSharedPtr<FJsonValue>>& Rows)
	{
		for (const TSharedPtr<FJsonValue>& Value : Rows)
		{
			const TSharedPtr<FJsonObject> Row = Value->AsObject();
			if (!Row.IsValid()) continue;
			const TSharedPtr<FJsonValue>* Feeds = Row->Values.Find(TEXT("feeds"));
			if (Feeds && (*Feeds)->Type == EJson::String)
			{
				TArray<TSharedPtr<FJsonValue>> Items;
				Items.Add(MakeShared<FJsonValueString>((*Feeds)->AsString()));
				Row->SetArrayField(TEXT("feeds"), Items);
			}
		}
	}

	bool NormaliseRungRows(TArray<TSharedPtr<FJsonValue>>& Rows)
	{
		for (const TSharedPtr<FJsonValue>& Value : Rows)
		{
			const TSharedPtr<FJsonObject> Row = Value->AsObject();
			if (!Row.IsValid()) return Fail(TEXT("Rung row is not an object."));
			const TSharedPtr<FJsonValue>* RestorationValue = Row->Values.Find(TEXT("restoration"));
			if (!RestorationValue) continue;
			const TSharedPtr<FJsonObject> Restoration = (*RestorationValue)->AsObject();
			if (!Restoration.IsValid()) return Fail(TEXT("Rung restoration is not an object."));
			const TSharedPtr<FJsonValue>* DepartureValue = Restoration->Values.Find(TEXT("departure"));
			if (!DepartureValue) return Fail(TEXT("Rung restoration is missing departure."));
			const TSharedPtr<FJsonObject> Departure = (*DepartureValue)->AsObject();
			if (!Departure.IsValid()) return Fail(TEXT("Rung departure is not an object."));
			Restoration->Values.Remove(TEXT("departure"));
			for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Departure->Values)
			{
				Restoration->SetField(FString(Pair.Key), Pair.Value);
			}
		}
		return true;
	}

	bool NormaliseUpgradeRows(TArray<TSharedPtr<FJsonValue>>& Rows)
	{
		for (const TSharedPtr<FJsonValue>& Value : Rows)
		{
			const TSharedPtr<FJsonObject> Row = Value->AsObject();
			if (!Row.IsValid()) return Fail(TEXT("Upgrade row is not an object."));
			const TSharedPtr<FJsonValue>* EffectValue = Row->Values.Find(TEXT("effect"));
			if (!EffectValue) return Fail(TEXT("Upgrade row is missing effect."));
			const TSharedPtr<FJsonObject> Effect = (*EffectValue)->AsObject();
			if (!Effect.IsValid()) return Fail(TEXT("Upgrade effect is not an object."));
			const TSharedPtr<FJsonValue>* Multiplier = Effect->Values.Find(TEXT("relicBurdenMultiplier"));
			if (!Multiplier) return Fail(TEXT("Upgrade effect is missing relicBurdenMultiplier."));
			Row->Values.Remove(TEXT("effect"));
			Row->Values.Add(TEXT("relicBurdenMultiplier"), *Multiplier);
		}
		return true;
	}

	bool SaveAsset(UObject* Asset);

	template <typename RowType>
	UDataTable* GetOrCreateTable(const FString& AssetName)
	{
		const FString PackageName = FString::Printf(TEXT("%s/%s"), AssetRoot, *AssetName);
		UPackage* Package = LoadPackage(nullptr, *PackageName, LOAD_None);
		if (!Package) Package = CreatePackage(*PackageName);
		UDataTable* Table = FindObject<UDataTable>(Package, *AssetName);
		if (!Table) Table = NewObject<UDataTable>(Package, *AssetName, RF_Public | RF_Standalone);
		Table->RowStruct = RowType::StaticStruct();
		Table->bIgnoreExtraFields = false;
		Table->bIgnoreMissingFields = true;
		return Table;
	}

	template <typename RowType>
	bool ImportTable(const FString& AssetName, const TArray<TSharedPtr<FJsonValue>>& Rows)
	{
		FString Json;
		if (!WriteJsonArray(Rows, Json)) return Fail(FString::Printf(TEXT("Could not serialise %s rows."), *AssetName));

		UDataTable* Table = GetOrCreateTable<RowType>(AssetName);
		const TArray<FString> Problems = Table->CreateTableFromJSONString(Json);
		if (!Problems.IsEmpty())
		{
			for (const FString& Problem : Problems) UE_LOG(LogLLKDataImport, Error, TEXT("%s: %s"), *AssetName, *Problem);
			return false;
		}
		return SaveAsset(Table);
	}

	bool ImportRungs(const TSharedPtr<FJsonObject>& Ledger)
	{
		const TArray<TSharedPtr<FJsonValue>>* SourceRows = nullptr;
		if (!GetArray(Ledger, TEXT("rungs"), SourceRows)) return false;
		UDataTable* Table = GetOrCreateTable<FLLKRungDef>(TEXT("DT_LedgerRungs"));
		Table->EmptyTable();
		for (const TSharedPtr<FJsonValue>& Value : *SourceRows)
		{
			const TSharedPtr<FJsonObject> Source = Value->AsObject();
			if (!Source.IsValid()) return Fail(TEXT("Rung row is not an object."));
			FString Id;
			FLLKRungDef Row;
			if (!GetString(Source, TEXT("id"), Id) || !GetString(Source, TEXT("title"), Row.Title)
				|| !GetString(Source, TEXT("job"), Row.Job) || !GetString(Source, TEXT("opens"), Row.Opens)
				|| !GetString(Source, TEXT("wakes"), Row.Wakes)) return false;
			const TSharedPtr<FJsonObject>* RestorationObject = nullptr;
			if (Source->TryGetObjectField(TEXT("restoration"), RestorationObject) && RestorationObject && RestorationObject->IsValid())
			{
				FLLKRungRestoration& Restoration = Row.Restoration.Emplace();
				FString RequiresRoom;
				if (!GetString(*RestorationObject, TEXT("requiresRoom"), RequiresRoom)) return false;
				Restoration.RequiresRoom = FName(RequiresRoom);
				TSharedPtr<FJsonObject> Salvage, Departure;
				if (!GetObject(*RestorationObject, TEXT("salvage"), Salvage)
					|| !GetObject(*RestorationObject, TEXT("departure"), Departure)) return false;
				for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Salvage->Values)
				{
					Restoration.Salvage.Add(FName(Pair.Key), static_cast<int32>(Pair.Value->AsNumber()));
				}
				const TArray<TSharedPtr<FJsonValue>>* Wakes = nullptr;
				if (!GetArray(*RestorationObject, TEXT("wakesWithoutLantern"), Wakes)
					|| !GetNumber(Departure, TEXT("unpoweredSeconds"), Restoration.UnpoweredSeconds)
					|| !GetNumber(Departure, TEXT("poweredSeconds"), Restoration.PoweredSeconds)
					|| !GetNumber(Departure, TEXT("unpoweredFalseStartChance"), Restoration.UnpoweredFalseStartChance)) return false;
				for (const TSharedPtr<FJsonValue>& Wake : *Wakes) Restoration.WakesWithoutLantern.Add(FName(Wake->AsString()));
			}
			Table->AddRow(FName(Id), Row);
		}
		return SaveAsset(Table);
	}

	template <typename AssetType>
	AssetType* GetOrCreateAsset(const FString& AssetName)
	{
		const FString PackageName = FString::Printf(TEXT("%s/%s"), AssetRoot, *AssetName);
		UPackage* Package = LoadPackage(nullptr, *PackageName, LOAD_None);
		if (!Package) Package = CreatePackage(*PackageName);
		AssetType* Asset = FindObject<AssetType>(Package, *AssetName);
		if (!Asset) Asset = NewObject<AssetType>(Package, *AssetName, RF_Public | RF_Standalone);
		return Asset;
	}

	bool SaveAsset(UObject* Asset)
	{
		UPackage* Package = Asset->GetOutermost();
		Asset->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(Asset);
		const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		if (!UPackage::SavePackage(Package, Asset, *Filename, Args))
		{
			return Fail(FString::Printf(TEXT("Failed to save %s"), *Filename));
		}
		return true;
	}

	bool VerifyTable(const FString& AssetName, const TArray<FName>& ExpectedRows)
	{
		const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), AssetRoot, *AssetName, *AssetName);
		UDataTable* Table = ::LoadObject<UDataTable>(nullptr, *ObjectPath);
		if (!Table) return Fail(FString::Printf(TEXT("Could not reload generated table %s."), *AssetName));
		const TArray<FName> ActualRows = Table->GetRowNames();
		if (ActualRows.Num() != ExpectedRows.Num())
		{
			return Fail(FString::Printf(TEXT("%s has %d rows; expected %d."), *AssetName, ActualRows.Num(), ExpectedRows.Num()));
		}
		for (const FName Name : ExpectedRows)
		{
			if (!ActualRows.Contains(Name)) return Fail(FString::Printf(TEXT("%s is missing row %s."), *AssetName, *Name.ToString()));
		}
		return true;
	}

	bool VerifyRuleAssets(const TSharedPtr<FJsonObject>& Storm, const TSharedPtr<FJsonObject>& Lantern, const TSharedPtr<FJsonObject>& Loot, const TSharedPtr<FJsonObject>& Workshop)
	{
		ULLKStormChargeData* StormAsset = ::LoadObject<ULLKStormChargeData>(nullptr, TEXT("/Game/Data/DA_StormCharge.DA_StormCharge"));
		ULLKLanternData* LanternAsset = ::LoadObject<ULLKLanternData>(nullptr, TEXT("/Game/Data/DA_Lantern.DA_Lantern"));
		ULLKLootRulesData* LootAsset = ::LoadObject<ULLKLootRulesData>(nullptr, TEXT("/Game/Data/DA_LootRules.DA_LootRules"));
		ULLKWorkshopRulesData* WorkshopAsset = ::LoadObject<ULLKWorkshopRulesData>(nullptr, TEXT("/Game/Data/DA_WorkshopRules.DA_WorkshopRules"));
		if (!StormAsset || !LanternAsset || !LootAsset || !WorkshopAsset) return Fail(TEXT("One or more generated rule assets could not be reloaded."));
		const TArray<TSharedPtr<FJsonValue>>* Thresholds = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Currency = nullptr;
		TSharedPtr<FJsonObject> Movement, Light, WarmColor, Extraction;
		double Baseline = 0.0;
		if (!Storm->TryGetNumberField(TEXT("baselineRatePerSecond"), Baseline) || !GetArray(Storm, TEXT("thresholds"), Thresholds)
			|| !GetObject(Lantern, TEXT("movement"), Movement) || !GetObject(Lantern, TEXT("light"), Light)
			|| !GetObject(Light, TEXT("warmColor"), WarmColor) || !GetObject(Loot, TEXT("extraction"), Extraction)
			|| !GetArray(Workshop, TEXT("currency"), Currency)) return false;
		double WalkSpeed = 0.0;
		double RadiusStuds = 0.0;
		double Intensity = 0.0;
		double WarmRed = 0.0;
		if (!Movement->TryGetNumberField(TEXT("walkSpeed"), WalkSpeed)) return false;
		if (!Light->TryGetNumberField(TEXT("radiusStuds"), RadiusStuds)
			|| !Light->TryGetNumberField(TEXT("intensity"), Intensity)
			|| !WarmColor->TryGetNumberField(TEXT("r"), WarmRed)) return false;
		if (!FMath::IsNearlyEqual(StormAsset->BaselineRatePerSecond, static_cast<float>(Baseline))
			|| StormAsset->Thresholds.Num() != Thresholds->Num()
			|| !FMath::IsNearlyEqual(LanternAsset->Movement.WalkSpeed, static_cast<float>(WalkSpeed))
			|| !FMath::IsNearlyEqual(LanternAsset->Light.RadiusStuds, static_cast<float>(RadiusStuds))
			|| !FMath::IsNearlyEqual(LanternAsset->Light.Intensity, static_cast<float>(Intensity))
			|| !FMath::IsNearlyEqual(LanternAsset->Light.WarmColor.R, static_cast<float>(WarmRed))
			|| LootAsset->Extraction.Num() != Extraction->Values.Num()
			|| WorkshopAsset->Currency.Num() != Currency->Num())
		{
			return Fail(TEXT("Generated rule assets do not semantically match their canonical payloads."));
		}
		return true;
	}

	TArray<FName> ObjectRowNames(const TSharedPtr<FJsonObject>& Source)
	{
		TArray<FName> Names;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Source->Values) Names.Add(FName(Pair.Key));
		return Names;
	}

	bool IdRowNames(const TArray<TSharedPtr<FJsonValue>>& Source, TArray<FName>& OutNames)
	{
		OutNames.Reset();
		for (const TSharedPtr<FJsonValue>& Value : Source)
		{
			const TSharedPtr<FJsonObject> Row = Value->AsObject();
			FString Id;
			if (!Row.IsValid() || !Row->TryGetStringField(TEXT("id"), Id)) return Fail(TEXT("Cannot read an expected row id."));
			OutNames.Add(FName(Id));
		}
		return true;
	}

	bool ReadBand(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, FLLKSecondsBand& OutBand)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!GetArray(Object, Field, Values) || Values->Num() != 2)
		{
			return Fail(FString::Printf(TEXT("'%s' must be a two-value seconds band."), Field));
		}
		OutBand.Low = static_cast<float>((*Values)[0]->AsNumber());
		OutBand.High = static_cast<float>((*Values)[1]->AsNumber());
		return true;
	}

	ELLKLanternCurve LanternCurve(const FString& Value)
	{
		return Value.Equals(TEXT("linearCapped"), ESearchCase::IgnoreCase) ? ELLKLanternCurve::LinearCapped : ELLKLanternCurve::Diminishing;
	}

	ELLKExtractionReliability ExtractionReliability(const FString& Value)
	{
		return Value.Equals(TEXT("risky"), ESearchCase::IgnoreCase) ? ELLKExtractionReliability::Risky : ELLKExtractionReliability::High;
	}

	bool ImportStorm(const TSharedPtr<FJsonObject>& Root)
	{
		ULLKStormChargeData* Asset = GetOrCreateAsset<ULLKStormChargeData>(TEXT("DA_StormCharge"));
		TSharedPtr<FJsonObject> Lantern, Run, Call, Warden, Anchor;
		if (!GetNumber(Root, TEXT("baselineRatePerSecond"), Asset->BaselineRatePerSecond)
			|| !GetObject(Root, TEXT("lantern"), Lantern) || !GetObject(Root, TEXT("run"), Run)
			|| !GetObject(Root, TEXT("extractionCall"), Call) || !GetObject(Root, TEXT("warden"), Warden)
			|| !GetObject(Root, TEXT("anchorRevive"), Anchor)) return false;

		FString Curve;
		if (!GetNumber(Lantern, TEXT("perLanternBonus"), Asset->Lantern.PerLanternBonus)
			|| !GetNumber(Lantern, TEXT("maxMultiplier"), Asset->Lantern.MaxMultiplier)
			|| !GetString(Lantern, TEXT("curve"), Curve)
			|| !GetNumber(Lantern, TEXT("diminishingDecay"), Asset->Lantern.DiminishingDecay)) return false;
		Asset->Lantern.Curve = LanternCurve(Curve);

		if (!GetNumber(Run, TEXT("earlyExitSeconds"), Asset->Run.EarlyExitSeconds)
			|| !GetNumber(Run, TEXT("targetSeconds"), Asset->Run.TargetSeconds)
			|| !ReadBand(Run, TEXT("targetBandSeconds"), Asset->Run.TargetBandSeconds)
			|| !GetNumber(Run, TEXT("hardCeilingSeconds"), Asset->Run.HardCeilingSeconds)) return false;

		if (!GetNumber(Call, TEXT("stormChargeSpike"), Asset->ExtractionCall.StormChargeSpike)
			|| !GetBool(Call, TEXT("bConvergeAllPatrols"), Asset->ExtractionCall.bConvergeAllPatrols)
			|| !GetNumber(Call, TEXT("sirenSeconds"), Asset->ExtractionCall.SirenSeconds)) return false;

		const TArray<TSharedPtr<FJsonValue>>* Thresholds = nullptr;
		if (!GetArray(Root, TEXT("thresholds"), Thresholds)) return false;
		Asset->Thresholds.Reset();
		for (const TSharedPtr<FJsonValue>& Value : *Thresholds)
		{
			const TSharedPtr<FJsonObject> Row = Value->AsObject();
			if (!Row.IsValid()) return Fail(TEXT("Storm threshold is not an object."));
			FLLKStormThreshold& Threshold = Asset->Thresholds.AddDefaulted_GetRef();
			FString Id;
			if (!GetString(Row, TEXT("id"), Id) || !GetNumber(Row, TEXT("value"), Threshold.Value)
				|| !GetString(Row, TEXT("effect"), Threshold.Effect)) return false;
			Threshold.Id = FName(Id);
			// These are writer-derived by contract; values in JSON are explanatory only.
			Threshold.DarkSeconds = Threshold.Value / Asset->BaselineRatePerSecond;
			Threshold.RegularUseSeconds = FMath::RoundToFloat(Threshold.Value / (Asset->BaselineRatePerSecond * (1.f + Asset->Lantern.PerLanternBonus * 0.5f)));
			Threshold.OneLanternSeconds = Threshold.Value / (Asset->BaselineRatePerSecond * (1.f + Asset->Lantern.PerLanternBonus));
		}

		if (!GetNumber(Warden, TEXT("chargeValue"), Asset->Warden.ChargeValue)
			|| !ReadBand(Warden, TEXT("arrivalBandSeconds"), Asset->Warden.ArrivalBandSeconds)
			|| !GetNumber(Warden, TEXT("lightSensitivity"), Asset->Warden.LightSensitivity)
			|| !GetString(Warden, TEXT("effect"), Asset->Warden.Effect)) return false;

		if (!GetInt(Anchor, TEXT("chargesPerRun"), Asset->AnchorRevive.ChargesPerRun)
			|| !GetNumber(Anchor, TEXT("reviveSeconds"), Asset->AnchorRevive.ReviveSeconds)
			|| !GetBool(Anchor, TEXT("bConsumesFullLanternCharge"), Asset->AnchorRevive.bConsumesFullLanternCharge)
			|| !GetNumber(Anchor, TEXT("stormChargeSpike"), Asset->AnchorRevive.StormChargeSpike)
			|| !GetBool(Anchor, TEXT("bSoloOnly"), Asset->AnchorRevive.bSoloOnly)) return false;
		return SaveAsset(Asset);
	}

	bool ImportLantern(const TSharedPtr<FJsonObject>& Root)
	{
		ULLKLanternData* Asset = GetOrCreateAsset<ULLKLanternData>(TEXT("DA_Lantern"));
		TSharedPtr<FJsonObject> Charge, Knockdown, Revive, Contact, Light, WarmColor, Movement;
		if (!GetObject(Root, TEXT("charge"), Charge) || !GetObject(Root, TEXT("knockdown"), Knockdown)
			|| !GetObject(Root, TEXT("revive"), Revive) || !GetObject(Root, TEXT("sweeperContact"), Contact)
			|| !GetObject(Root, TEXT("light"), Light) || !GetObject(Light, TEXT("warmColor"), WarmColor)
			|| !GetObject(Root, TEXT("movement"), Movement)) return false;
		if (!GetNumber(Charge, TEXT("secondsOfLightPerFullCharge"), Asset->Charge.SecondsOfLightPerFullCharge)
			|| !GetBool(Charge, TEXT("bAutoOffAtEmpty"), Asset->Charge.bAutoOffAtEmpty)
			|| !GetBool(Charge, TEXT("bRechargeAtRelay"), Asset->Charge.bRechargeAtRelay)
			|| !GetNumber(Knockdown, TEXT("downedWalkSpeed"), Asset->Knockdown.DownedWalkSpeed)
			|| !GetBool(Knockdown, TEXT("bLanternForcedOffWhenDowned"), Asset->Knockdown.bLanternForcedOffWhenDowned)
			|| !GetBool(Knockdown, TEXT("bDropsCarriedCrate"), Asset->Knockdown.bDropsCarriedCrate)
			|| !GetNumber(Knockdown, TEXT("bleedOutSeconds"), Asset->Knockdown.BleedOutSeconds)
			|| !GetNumber(Revive, TEXT("teammateHoldSeconds"), Asset->Revive.TeammateHoldSeconds)
			|| !GetNumber(Revive, TEXT("maxReviveDistance"), Asset->Revive.MaxReviveDistance)
			|| !GetBool(Contact, TEXT("bKnockdownOnContact"), Asset->SweeperContact.bKnockdownOnContact)
			|| !GetNumber(Contact, TEXT("contactRange"), Asset->SweeperContact.ContactRange)
			|| !GetNumber(Contact, TEXT("contactCooldownSeconds"), Asset->SweeperContact.ContactCooldownSeconds)
			|| !GetInt(Contact, TEXT("hitsToKnockDown"), Asset->SweeperContact.HitsToKnockDown)
			|| !GetNumber(Contact, TEXT("hitDecaySeconds"), Asset->SweeperContact.HitDecaySeconds)
			|| !GetNumber(Light, TEXT("radiusStuds"), Asset->Light.RadiusStuds)
			|| !GetNumber(Light, TEXT("coneAngleDegrees"), Asset->Light.ConeAngleDegrees)
			|| !GetNumber(Light, TEXT("intensity"), Asset->Light.Intensity)
			|| !GetNumber(WarmColor, TEXT("r"), Asset->Light.WarmColor.R)
			|| !GetNumber(WarmColor, TEXT("g"), Asset->Light.WarmColor.G)
			|| !GetNumber(WarmColor, TEXT("b"), Asset->Light.WarmColor.B)
			|| !GetBool(Light, TEXT("bCastsShadows"), Asset->Light.bCastsShadows)
			|| !GetNumber(Movement, TEXT("walkSpeed"), Asset->Movement.WalkSpeed)
			|| !GetNumber(Movement, TEXT("sprintSpeed"), Asset->Movement.SprintSpeed)
			|| !GetNumber(Movement, TEXT("crouchSpeed"), Asset->Movement.CrouchSpeed)) return false;
		return SaveAsset(Asset);
	}

	bool ImportLoot(const TSharedPtr<FJsonObject>& Root)
	{
		ULLKLootRulesData* Asset = GetOrCreateAsset<ULLKLootRulesData>(TEXT("DA_LootRules"));
		TSharedPtr<FJsonObject> Rules, Burden, Banking, Extraction;
		if (!GetObject(Root, TEXT("rules"), Rules) || !GetObject(Root, TEXT("burden"), Burden)
			|| !GetObject(Root, TEXT("banking"), Banking) || !GetObject(Root, TEXT("extraction"), Extraction)) return false;
		if (!GetBool(Rules, TEXT("bBaselineLootAlwaysVisible"), Asset->Rules.bBaselineLootAlwaysVisible)
			|| !GetBool(Rules, TEXT("bRelicsRequireLanternLight"), Asset->Rules.bRelicsRequireLanternLight)
			|| !GetBool(Rules, TEXT("bRestorationCratesArePocketable"), Asset->Rules.bRestorationCratesArePocketable)
			|| !GetNumber(Burden, TEXT("speedPenaltyPerPoint"), Asset->Burden.SpeedPenaltyPerPoint)
			|| !GetNumber(Burden, TEXT("maxSpeedPenalty"), Asset->Burden.MaxSpeedPenalty)
			|| !GetInt(Banking, TEXT("relayBanksPerRun"), Asset->Banking.RelayBanksPerRun)
			|| !GetNumber(Banking, TEXT("bankedKeptOnWipe"), Asset->Banking.BankedKeptOnWipe)
			|| !GetNumber(Banking, TEXT("unbankedKeptOnWipe"), Asset->Banking.UnbankedKeptOnWipe)
			|| !GetBool(Banking, TEXT("bDisconnectUsesSameRule"), Asset->Banking.bDisconnectUsesSameRule)) return false;

		Asset->Extraction.Reset();
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Extraction->Values)
		{
			const TSharedPtr<FJsonObject> Source = Pair.Value->AsObject();
			if (!Source.IsValid()) return Fail(TEXT("Extraction entry is not an object."));
			FLLKExtractionPoint& Point = Asset->Extraction.AddDefaulted_GetRef();
			FString Reliability, Requires;
			if (!GetString(Source, TEXT("reliability"), Reliability)
				|| !GetNumber(Source, TEXT("payoutModifier"), Point.PayoutModifier)
				|| !GetNumber(Source, TEXT("arrivalSeconds"), Point.ArrivalSeconds)
				|| !GetString(Source, TEXT("note"), Point.Note)) return false;
			Point.Id = FName(Pair.Key);
			Point.Reliability = ExtractionReliability(Reliability);
			if (Source->TryGetStringField(TEXT("requires"), Requires)) Point.Requires = FName(Requires);
		}
		return SaveAsset(Asset);
	}

	bool ImportWorkshop(const TSharedPtr<FJsonObject>& Root)
	{
		ULLKWorkshopRulesData* Asset = GetOrCreateAsset<ULLKWorkshopRulesData>(TEXT("DA_WorkshopRules"));
		const TArray<TSharedPtr<FJsonValue>>* Currency = nullptr;
		if (!GetArray(Root, TEXT("currency"), Currency)) return false;
		Asset->Currency.Reset();
		for (const TSharedPtr<FJsonValue>& Value : *Currency) Asset->Currency.Add(FName(Value->AsString()));
		return SaveAsset(Asset);
	}

	bool VerifyManifest(const FString& Filename)
	{
		TSharedPtr<FJsonObject> Manifest;
		if (!LoadObject(Filename, Manifest)) return false;
		const TArray<TSharedPtr<FJsonValue>>* Tables = nullptr;
		const TArray<TSharedPtr<FJsonValue>>* Assets = nullptr;
		return GetArray(Manifest, TEXT("DataTables"), Tables) && Tables->Num() == 7
			&& GetArray(Manifest, TEXT("DataAssets"), Assets) && Assets->Num() == 4;
	}
}

ULLKImportDataCommandlet::ULLKImportDataCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 ULLKImportDataCommandlet::Main(const FString& Params)
{
	using namespace LLKDataImport;
	FString InputDir = FPaths::Combine(FPaths::ProjectIntermediateDir(), TEXT("LLKDataImport"));
	FParse::Value(*Params, TEXT("InputDir="), InputDir);
	InputDir = FPaths::ConvertRelativePathToFull(InputDir);
	const FString UnrealDir = FPaths::Combine(InputDir, TEXT("unreal"));
	UE_LOG(LogLLKDataImport, Display, TEXT("LLK data import from %s"), *UnrealDir);

	if (!VerifyManifest(FPaths::Combine(InputDir, TEXT("asset-manifest.json")))) return 1;
	TSharedPtr<FJsonObject> Enemies, Loot, Ledger, Relics, Workshop, Town, Storm, Lantern;
	if (!LoadObject(FPaths::Combine(UnrealDir, TEXT("enemies.json")), Enemies)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("loot.json")), Loot)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("ledger.json")), Ledger)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("relics.json")), Relics)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("workshop.json")), Workshop)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("town.json")), Town)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("storm-charge.json")), Storm)
		|| !LoadObject(FPaths::Combine(UnrealDir, TEXT("lantern.json")), Lantern)) return 1;

	TArray<TSharedPtr<FJsonValue>> Rows;
	if (!BuildObjectRows(Enemies, Rows, true) || !ImportTable<FLLKEnemyDef>(TEXT("DT_Enemies"), Rows)) return 1;
	Rows.Reset();
	TSharedPtr<FJsonObject> Tiers;
	if (!GetObject(Loot, TEXT("tiers"), Tiers) || !BuildObjectRows(Tiers, Rows)) return 1;
	NormaliseLootTierRows(Rows);
	if (!ImportTable<FLLKLootTierDef>(TEXT("DT_LootTiers"), Rows)) return 1;
	Rows.Reset();
	const TArray<TSharedPtr<FJsonValue>>* SourceRows = nullptr;
	if (!GetArray(Ledger, TEXT("rooms"), SourceRows) || !BuildIdRows(*SourceRows, Rows) || !ImportTable<FLLKRoomDef>(TEXT("DT_LedgerRooms"), Rows)) return 1;
	Rows.Reset();
	if (!ImportRungs(Ledger)) return 1;
	Rows.Reset();
	if (!GetArray(Relics, TEXT("relics"), SourceRows) || !BuildIdRows(*SourceRows, Rows) || !ImportTable<FLLKRelicDef>(TEXT("DT_Relics"), Rows)) return 1;
	Rows.Reset();
	if (!GetArray(Workshop, TEXT("upgrades"), SourceRows) || !BuildIdRows(*SourceRows, Rows) || !NormaliseUpgradeRows(Rows) || !ImportTable<FLLKUpgradeDef>(TEXT("DT_WorkshopUpgrades"), Rows)) return 1;
	Rows.Reset();
	if (!GetArray(Town, TEXT("landmarks"), SourceRows) || !BuildIdRows(*SourceRows, Rows) || !ImportTable<FLLKLandmarkDef>(TEXT("DT_TownLandmarks"), Rows)) return 1;

	if (!ImportStorm(Storm) || !ImportLantern(Lantern) || !ImportLoot(Loot) || !ImportWorkshop(Workshop)) return 1;

	TArray<FName> ExpectedIds;
	if (!VerifyTable(TEXT("DT_Enemies"), ObjectRowNames(Enemies))
		|| !VerifyTable(TEXT("DT_LootTiers"), ObjectRowNames(Tiers))
		|| !GetArray(Ledger, TEXT("rooms"), SourceRows) || !IdRowNames(*SourceRows, ExpectedIds) || !VerifyTable(TEXT("DT_LedgerRooms"), ExpectedIds)
		|| !GetArray(Ledger, TEXT("rungs"), SourceRows) || !IdRowNames(*SourceRows, ExpectedIds) || !VerifyTable(TEXT("DT_LedgerRungs"), ExpectedIds)
		|| !GetArray(Relics, TEXT("relics"), SourceRows) || !IdRowNames(*SourceRows, ExpectedIds) || !VerifyTable(TEXT("DT_Relics"), ExpectedIds)
		|| !GetArray(Workshop, TEXT("upgrades"), SourceRows) || !IdRowNames(*SourceRows, ExpectedIds) || !VerifyTable(TEXT("DT_WorkshopUpgrades"), ExpectedIds)
		|| !GetArray(Town, TEXT("landmarks"), SourceRows) || !IdRowNames(*SourceRows, ExpectedIds) || !VerifyTable(TEXT("DT_TownLandmarks"), ExpectedIds)
		|| !VerifyRuleAssets(Storm, Lantern, Loot, Workshop)) return 1;
	UE_LOG(LogLLKDataImport, Display, TEXT("LLK data import completed: 7 DataTables and 4 DataAssets."));
	return 0;
}
