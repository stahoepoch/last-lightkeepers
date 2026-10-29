using UnrealBuildTool;

public class LLKEditor : ModuleRules
{
    public LLKEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "LLK",
            "UnrealEd",
            "Json",
            "JsonUtilities",
            "AssetRegistry",
            "EnhancedInput",
            "InputCore"
        });
    }
}
