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
            // Map-readiness navigation diagnostics.  The runtime contract owns
            // the observations; this module supplies the read-only query API.
            "NavigationSystem",
            // EKeys lives here. The monolithic Game target links without it
            // and the modular Editor target does not, so leaving it out gives
            // a clean build that fails the moment the editor is rebuilt.
            "InputCore"
        });
    }
}
