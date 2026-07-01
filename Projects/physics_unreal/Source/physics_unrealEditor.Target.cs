// Target de build do editor (é o que roda ao abrir o projeto no UE Editor).
using UnrealBuildTool;
using System.Collections.Generic;

public class physics_unrealEditorTarget : TargetRules
{
	public physics_unrealEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("physics_unreal");
	}
}
