using UnrealBuildTool;

public class LLK : ModuleRules
{
    public LLK(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "EnhancedInput"
        });
    }
}
