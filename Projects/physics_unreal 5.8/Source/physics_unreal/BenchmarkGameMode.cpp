#include "BenchmarkGameMode.h"
#include "BenchmarkSettings.h"
#include "BenchmarkCommon.h"
#include "TowerBenchmark.h"
#include "RainBenchmark.h"
#include "Engine/World.h"

ABenchmarkGameMode::ABenchmarkGameMode()
{
	// Sem pawn/HUD/controller custom - a cena e 100% gerada pelos atores.
	DefaultPawnClass = nullptr;
}

void ABenchmarkGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	BenchmarkUtil::ApplyGlobalSettings(World);

	const EBenchScenario Scenario = UBenchmarkSettings::ResolveScenario();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (Scenario == EBenchScenario::Rain)
	{
		World->SpawnActor<ARainBenchmark>(ARainBenchmark::StaticClass(), FTransform::Identity, Params);
		UE_LOG(LogTemp, Warning, TEXT("[Benchmark] Bootstrap -> cenario=Chuva, saida=%s"),
			*BenchmarkUtil::ResolveOutputRoot());
	}
	else
	{
		World->SpawnActor<ATowerBenchmark>(ATowerBenchmark::StaticClass(), FTransform::Identity, Params);
		UE_LOG(LogTemp, Warning, TEXT("[Benchmark] Bootstrap -> cenario=Torre, saida=%s"),
			*BenchmarkUtil::ResolveOutputRoot());
	}
}
