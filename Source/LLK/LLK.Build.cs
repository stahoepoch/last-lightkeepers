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
            "EnhancedInput",
            // EKeys lives here. The monolithic Game target links without it
            // and the modular Editor target does not, so leaving it out gives
            // a clean build that fails the moment the editor is rebuilt.
            "InputCore"
        });
    }
}
