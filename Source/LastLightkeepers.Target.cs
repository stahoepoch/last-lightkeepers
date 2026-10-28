using UnrealBuildTool;
using System.Collections.Generic;

public class LastLightkeepersTarget : TargetRules
{
    public LastLightkeepersTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("LLK");
    }
}
