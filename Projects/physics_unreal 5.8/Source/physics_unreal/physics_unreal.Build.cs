// Regras de build do módulo primário do jogo.
using UnrealBuildTool;

public class physics_unreal : ModuleRules
{
	public physics_unreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Dependências: Engine (atores/componentes/mesh), PhysicsCore + Chaos
		// (materiais físicos, solver), DeveloperSettings (UBenchmarkSettings que
		// aparece em Project Settings), Projects (resolução de caminhos).
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"PhysicsCore",
			"Chaos",
			"DeveloperSettings",
			"Projects"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
