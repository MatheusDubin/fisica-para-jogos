#include "RainBenchmark.h"
#include "BenchmarkCommon.h"
#include "BenchmarkSettings.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "HAL/PlatformTime.h"
#include "Math/RandomStream.h"
#include "Kismet/KismetSystemLibrary.h"

static const FString RAIN_HEADER =
	TEXT("run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras");

ARainBenchmark::ARainBenchmark()
{
}

void ARainBenchmark::BeginPlay()
{
	Super::BeginPlay();

	const UBenchmarkSettings* S = GetDefault<UBenchmarkSettings>();
	Variations = S->RainVariations;
	if (Variations.Num() == 0)
	{
		Variations = { 1000, 5000, 10000 };
	}
	RunsPerVar = FMath::Max(1, S->RainRuns);
	Subdir = S->RainSubdir.IsEmpty() ? TEXT("chuva-default") : S->RainSubdir;
	bDrawWalls = S->bDrawWalls;
	WarmupSteps = FMath::Max(0, S->RainWarmupSteps);
	WindowSteps = FMath::Max(1, S->RainWindowSteps);

	// Coluna exploratória "Chaos-otimizado": aplica o bundle de CVars e grava em
	// pasta separada. Com bRainOptimize=false, reseta as CVars aos padrões, então
	// um run default fica limpo mesmo depois de um otimizado (não sobrescreve
	// chuva-default; a coluna default continua re-rodável).
	BenchmarkUtil::ApplyChaosCVars(GetWorld(), S->bRainOptimize);
	if (S->bRainOptimize)
	{
		Subdir = TEXT("chuva-optimized");
	}

	SphereMesh = BenchmarkUtil::LoadSphereMesh();
	SphereScale = BenchmarkUtil::UniformScaleForHalfExtent(SphereMesh, SPHERE_RADIUS);
	// Chuva: atrito 0.4, restituicao 0.3, bounce combine Average.
	Mat = BenchmarkUtil::MakePhysMaterial(this, /*Friction=*/0.4f, /*Restitution=*/0.3f, /*bBounceMinimum=*/false);

	CsvPath = BenchmarkUtil::PrepareFile(Subdir, TEXT("chuva_unreal.csv"));
	BenchmarkUtil::WriteHeader(CsvPath, RAIN_HEADER);

	BuildBox();
	BenchmarkUtil::CreateViewer(GetWorld(),
		FVector(5500, 5500, 3500),
		FVector(0, 0, 1800));

	UE_LOG(LogTemp, Warning, TEXT("[Chuva] Unreal/Chaos | variacoes=[%s] runs/var=%d | saida=%s"),
		*FString::JoinBy(Variations, TEXT(","), [](int32 V){ return FString::FromInt(V); }),
		RunsPerVar, *CsvPath);

	StartRun();
}

void ARainBenchmark::BuildBox()
{
	const float Largura = 3500.0f, Altura = 8000.0f, Profundidade = 3500.0f, Esp = 200.0f;
	const float MeioZ = Altura * 0.5f;
	UWorld* W = GetWorld();

	// So o chao e visivel (paridade com Godot/Unity).
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(0, 0, -Esp * 0.5f), FVector(Largura, Profundidade, Esp), true); // chao (sempre visivel)
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(0, 0, Altura + Esp * 0.5f), FVector(Largura, Profundidade, Esp), bDrawWalls);
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(Largura * 0.5f + Esp * 0.5f, 0, MeioZ), FVector(Esp, Profundidade, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(-Largura * 0.5f - Esp * 0.5f, 0, MeioZ), FVector(Esp, Profundidade, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(0, Profundidade * 0.5f + Esp * 0.5f, MeioZ), FVector(Largura, Esp, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, BenchmarkUtil::LoadCubeMesh(), Mat, FVector(0, -Profundidade * 0.5f - Esp * 0.5f, MeioZ), FVector(Largura, Esp, Altura), bDrawWalls);
}

void ARainBenchmark::StartRun()
{
	StepSamples.Reset();
	FpsSamples.Reset();
	bWarming = true;
	bCollecting = false;
	bRunning = true;
	StepsSinceSpawn = 0;

	const double t0 = FPlatformTime::Seconds();
	SpawnSpheres();
	tStart = FPlatformTime::Seconds();
	tLastFrame = tStart;

	UE_LOG(LogTemp, Warning, TEXT("[Chuva] Variacao=%d Run %d/%d (spawn %d esferas em %.0f ms)"),
		CurrentVar(), Run + 1, RunsPerVar, Spheres.Num(), (tStart - t0) * 1000.0);
}

void ARainBenchmark::SpawnSpheres()
{
	const int32 N = CurrentVar();
	const int32 PorLado = FMath::Max(1, FMath::CeilToInt(FMath::Pow((float)N, 1.0f / 3.0f)));
	const float OrigemX = -(PorLado - 1) * SPACING * 0.5f;
	const float OrigemY = -(PorLado - 1) * SPACING * 0.5f;

	FRandomStream Rng(0xBEEF + Run); // seed deterministico por run

	Spheres.Reset();
	int32 n = 0;
	for (int32 ix = 0; ix < PorLado && n < N; ix++)
	{
		for (int32 iz = 0; iz < PorLado && n < N; iz++)
		{
			for (int32 iy = 0; iy < PorLado && n < N; iy++)
			{
				if (n >= N) { break; }
				const FVector Jitter(
					Rng.FRandRange(-3.0f, 3.0f),
					Rng.FRandRange(-3.0f, 3.0f),
					Rng.FRandRange(-3.0f, 3.0f));
				const FVector Pos(
					OrigemX + ix * SPACING,
					OrigemY + iy * SPACING,
					SPAWN_Z + iz * SPACING);
				UStaticMeshComponent* C = BenchmarkUtil::SpawnDynamicBody(
					GetWorld(), SphereMesh, Mat, Pos + Jitter, SphereScale, /*bCastShadow=*/false);
				if (C) { Spheres.Add(C); }
				n++;
			}
		}
	}
}

void ARainBenchmark::PostPhysicsStep(float StepMs, float DeltaSeconds)
{
	if (bFinished)
	{
		return;
	}

	if (!bRunning)
	{
		if (ResetCountdown > 0)
		{
			ResetCountdown--;
			return;
		}
		StartRun();
		return;
	}

	const double Now = FPlatformTime::Seconds();
	const double RealDt = Now - tLastFrame;
	tLastFrame = Now;

	StepsSinceSpawn++;

	if (bWarming)
	{
		// Aquecimento por TEMPO DE SIMULACAO: espera as esferas cairem e
		// comecarem a empilhar (WARMUP_STEPS passos), independentemente do FPS
		// real. Sem isto, N alto media so a queda livre (poucos contatos).
		if (StepsSinceSpawn >= WarmupSteps)
		{
			bWarming = false;
			bCollecting = true;
			tCollect = Now;
		}
		return;
	}

	if (bCollecting)
	{
		StepSamples.Add(StepMs);
		if (RealDt > 0.0)
		{
			FpsSamples.Add((float)(1.0 / RealDt));
		}

		if (StepSamples.Num() % 100 == 1)
		{
			const double SimT = StepSamples.Num() * 0.02;
			UE_LOG(LogTemp, Warning, TEXT("[Chuva %d] coleta sim_t=%.1fs step=%.2fms fps=%.1f n=%d"),
				CurrentVar(), SimT, StepMs, RealDt > 0.0 ? 1.0 / RealDt : 0.0, StepSamples.Num());
		}
		// Janela por TEMPO DE SIMULACAO: WindowSteps passos (default 500 = 10s sim).
		if (StepSamples.Num() >= WindowSteps)
		{
			FinalizeRun();
		}
	}
}

void ARainBenchmark::FinalizeRun()
{
	bRunning = false;
	bCollecting = false;

	float StepMean = 0.0f, StepMax = 0.0f, FpsMean = 0.0f;
	for (float V : StepSamples) { StepMean += V; if (V > StepMax) { StepMax = V; } }
	StepMean = StepSamples.Num() > 0 ? StepMean / StepSamples.Num() : 0.0f;
	for (float V : FpsSamples) { FpsMean += V; }
	FpsMean = FpsSamples.Num() > 0 ? FpsMean / FpsSamples.Num() : 0.0f;
	const int32 Amostras = StepSamples.Num();

	BenchmarkUtil::AppendLine(CsvPath, FString::Printf(
		TEXT("%d,%d,%.4f,%.4f,%.2f,%d"),
		Run + 1, CurrentVar(), StepMean, StepMax, FpsMean, Amostras));

	const double WallDur = FPlatformTime::Seconds() - tCollect; // quanto os 10s simulados levaram em tempo real
	UE_LOG(LogTemp, Warning, TEXT("[Chuva %d] Run %d/%d step=%.3fms (max %.3f) fps=%.1f amostras=%d (janela 10s sim = %.1fs reais)"),
		CurrentVar(), Run + 1, RunsPerVar, StepMean, StepMax, FpsMean, Amostras, WallDur);

	// Reset in-place: destroi as esferas. Espera mais frames em N alto para o
	// Chaos limpar 10k bodies antes do proximo spawn.
	for (UStaticMeshComponent* C : Spheres)
	{
		if (C && C->GetOwner()) { C->GetOwner()->Destroy(); }
	}
	Spheres.Reset();
	ResetCountdown = 5;

	Run++;
	if (Run >= RunsPerVar)
	{
		Run = 0;
		VarIdx++;
		if (VarIdx >= Variations.Num())
		{
			Finish();
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[Chuva] Avancando para variacao=%d"), CurrentVar());
	}
}

void ARainBenchmark::Finish()
{
	bFinished = true;
	bRunning = false;
	UE_LOG(LogTemp, Warning, TEXT("[Chuva] CONCLUIDO. Resultados em %s"), *CsvPath);
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
