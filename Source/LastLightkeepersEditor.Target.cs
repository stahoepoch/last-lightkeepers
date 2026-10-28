using UnrealBuildTool;
using System.Collections.Generic;

public class LastLightkeepersEditorTarget : TargetRules
{
    public LastLightkeepersEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "LLK", "LLKEditor" });
    }
}
