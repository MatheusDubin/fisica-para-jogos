// Target de build do jogo (Development/Shipping standalone).
using UnrealBuildTool;
using System.Collections.Generic;

public class physics_unrealTarget : TargetRules
{
	public physics_unrealTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("physics_unreal");
	}
}
