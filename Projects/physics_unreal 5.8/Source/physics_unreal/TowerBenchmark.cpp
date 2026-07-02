#include "TowerBenchmark.h"
#include "BenchmarkCommon.h"
#include "BenchmarkSettings.h"

#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraActor.h"

static const FString CSV_HEADER =
	TEXT("run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep");
static const FString DEBUG_HEADER =
	TEXT("run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y");

ATowerBenchmark::ATowerBenchmark()
{
}

void ATowerBenchmark::BeginPlay()
{
	Super::BeginPlay();

	// --- Resolve config a partir das settings ---
	const UBenchmarkSettings* S = GetDefault<UBenchmarkSettings>();
	NValues = S->TowerN;
	if (NValues.Num() == 0)
	{
		NValues.Add(100);
	}
	RunsPerN = FMath::Max(1, S->TowerRunsPerN);
	Subdir = S->TowerSubdir.IsEmpty() ? TEXT("torre-N100-arena") : S->TowerSubdir;
	bDrawWalls = S->bDrawWalls;

	// --- Recursos compartilhados ---
	CubeMesh = BenchmarkUtil::LoadCubeMesh();
	CubeScale = BenchmarkUtil::UniformScaleForHalfExtent(CubeMesh, CUBE_HALF);
	// Torre: atrito 0.5, restituicao 0.0 (critico), bounce combine Minimum.
	Mat = BenchmarkUtil::MakePhysMaterial(this, /*Friction=*/0.5f, /*Restitution=*/0.0f, /*bBounceMinimum=*/true);

	// --- Arquivos de saida ---
	CsvPath = BenchmarkUtil::PrepareFile(Subdir, TEXT("torre_unreal.csv"));
	DebugPath = BenchmarkUtil::PrepareFile(Subdir, TEXT("torre_unreal_debug.csv"));
	BenchmarkUtil::WriteHeader(CsvPath, CSV_HEADER);
	BenchmarkUtil::WriteHeader(DebugPath, DEBUG_HEADER);

	BuildArena();

	// Cria luzes + camera uma vez; a camera e REENQUADRADA por N em cada run
	// (senao, no sweep, ela fica longe demais para os N pequenos).
	Cam = BenchmarkUtil::CreateViewer(GetWorld(),
		FVector(1000.0f, 350.0f, 550.0f), FVector(0.0f, 0.0f, 500.0f));

	const UPhysicsSettings* PS = UPhysicsSettings::Get();
	UE_LOG(LogTemp, Warning,
		TEXT("[Torre] Unreal/Chaos | substep=%d maxSubDt=%.4f maxSubsteps=%d | N=[%s] runs/N=%d | saida=%s"),
		PS ? (PS->bSubstepping ? 1 : 0) : -1,
		PS ? PS->MaxSubstepDeltaTime : -1.0f,
		PS ? PS->MaxSubsteps : -1,
		*FString::JoinBy(NValues, TEXT(","), [](int32 V){ return FString::FromInt(V); }),
		RunsPerN, *CsvPath);

	StartRun();
}

void ATowerBenchmark::BuildArena()
{
	int32 MaxN = 1;
	for (int32 V : NValues) { MaxN = FMath::Max(MaxN, V); }

	const float Largura = 6000.0f, Profundidade = 6000.0f, Esp = 200.0f;
	const float Altura = FMath::Max(15000.0f, MaxN * SPACING * 1.5f);
	const float MeioZ = Altura * 0.5f;
	UWorld* W = GetWorld();

	// Apenas o chao e visivel; paredes e teto sao collision-only (igual ao
	// Godot/Unity) para a camera externa ver os cubos e o FPS nao incluir o
	// custo de renderizar paredes que as outras engines nao desenham.
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(0, 0, -Esp * 0.5f), FVector(Largura, Profundidade, Esp), true);   // chao (sempre visivel)
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(0, 0, Altura + Esp * 0.5f), FVector(Largura, Profundidade, Esp), bDrawWalls); // teto
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(Largura * 0.5f + Esp * 0.5f, 0, MeioZ), FVector(Esp, Profundidade, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(-Largura * 0.5f - Esp * 0.5f, 0, MeioZ), FVector(Esp, Profundidade, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(0, Profundidade * 0.5f + Esp * 0.5f, MeioZ), FVector(Largura, Esp, Altura), bDrawWalls);
	BenchmarkUtil::CreateWall(W, CubeMesh, Mat, FVector(0, -Profundidade * 0.5f - Esp * 0.5f, MeioZ), FVector(Largura, Esp, Altura), bDrawWalls);
}

// Reposiciona a camera para enquadrar uma torre de N cubos (altura = N*100 cm).
// Vista 3/4 a CAM_DIST_FACTOR * altura de distancia, olhando para o meio da pilha.
void ATowerBenchmark::FrameTowerCamera(int32 N)
{
	if (!Cam)
	{
		return;
	}
	// Enquadramento calculado: FOV ~60 deg vertical (default 90 deg horizontal @16:9),
	// vista 3/4 (azimute 45 deg -> X e Y iguais). Distancia horizontal por eixo =
	// 0.81*H enquadra a torre inteira ocupando ~80% da altura do frame. Mira em
	// 0.42*H (um pouco abaixo do centro, para o colapso perto do chao ficar visivel).
	const float H = N * SPACING; // altura da torre em cm
	const FVector Look(0.0f, 0.0f, H * 0.42f);
	const float D = FMath::Max(1500.0f, H * CAM_DIST_FACTOR);
	const FVector Loc(D, D, H * 0.50f);
	Cam->SetActorLocation(Loc);
	Cam->SetActorRotation((Look - Loc).Rotation());
}

void ATowerBenchmark::StartRun()
{
	SimElapsed = 0.0f;
	PhysFrames = 0;
	RunMaxV = 0.0f;
	tFirstSleep = -1.0f;
	tHalfSleep = -1.0f;
	DebugBuffer.Reset();

	const int32 N = CurrentN();
	FrameTowerCamera(N);
	Bodies.Reset();
	for (int32 i = 0; i < N; i++)
	{
		const float Z = OFFSET_INICIAL + i * SPACING;
		UStaticMeshComponent* C = BenchmarkUtil::SpawnDynamicBody(
			GetWorld(), CubeMesh, Mat, FVector(0, 0, Z), CubeScale, /*bCastShadow=*/false);
		if (C) { Bodies.Add(C); }
	}

	bRunning = true;
	UE_LOG(LogTemp, Warning, TEXT("[Torre] N=%d Run %d/%d iniciada (%d cubos)"),
		N, Run + 1, RunsPerN, Bodies.Num());
}

void ATowerBenchmark::PostPhysicsStep(float StepMs, float DeltaSeconds)
{
	if (bFinished)
	{
		return;
	}

	if (!bRunning)
	{
		// Espera alguns frames apos destruir os corpos antes do proximo spawn
		// (deixa o Chaos limpar os bodies destruidos).
		if (ResetCountdown > 0)
		{
			ResetCountdown--;
			return;
		}
		StartRun();
		return;
	}

	PhysFrames++;
	SimElapsed += DeltaSeconds;

	const int32 N = Bodies.Num();
	int32 Asleep = 0;
	float MaxVcm = 0.0f, SumVcm = 0.0f, TopZcm = -FLT_MAX;
	for (UStaticMeshComponent* C : Bodies)
	{
		if (!C) { continue; }
		if (!C->RigidBodyIsAwake()) { Asleep++; }
		const float V = C->GetPhysicsLinearVelocity().Size();
		if (V > MaxVcm) { MaxVcm = V; }
		SumVcm += V;
		const float Z = C->GetComponentLocation().Z;
		if (Z > TopZcm) { TopZcm = Z; }
	}

	// Converte cm -> m para o CSV (comparavel com Godot/Unity).
	const float MaxV = MaxVcm / 100.0f;
	const float MeanV = (N > 0 ? SumVcm / N : 0.0f) / 100.0f;
	const float TopY = TopZcm / 100.0f;

	if (MaxV > RunMaxV) { RunMaxV = MaxV; }
	if (Asleep > 0 && tFirstSleep < 0.0f) { tFirstSleep = SimElapsed; }
	if (Asleep >= N / 2 && tHalfSleep < 0.0f) { tHalfSleep = SimElapsed; }

	DebugBuffer.Add(FString::Printf(TEXT("%d,%d,%.4f,%d,%d,%d,%.6f,%.6f,%.4f"),
		Run + 1, N, SimElapsed, PhysFrames, Asleep, N - Asleep, MaxV, MeanV, TopY));

	const bool bAllSleep = (Asleep == N);
	const bool bTimeout = (SimElapsed >= TIMEOUT_S);
	if (bAllSleep || bTimeout)
	{
		RecordRun(bTimeout, Asleep);
		EndRunAndAdvance();
	}
}

void ATowerBenchmark::RecordRun(bool bTimeout, int32 Asleep)
{
	const int32 N = Bodies.Num();
	BenchmarkUtil::AppendLine(CsvPath, FString::Printf(
		TEXT("%d,%d,%.4f,%d,%d,%.6f,%.4f,%.4f"),
		Run + 1, N, SimElapsed, bTimeout ? 1 : 0, PhysFrames, RunMaxV, tFirstSleep, tHalfSleep));

	BenchmarkUtil::AppendLines(DebugPath, DebugBuffer);

	UE_LOG(LogTemp, Warning, TEXT("[Torre] N=%d Run %d/%d -> %.3fs (frames=%d, max_v=%.2f, dormindo=%d/%d)%s"),
		N, Run + 1, RunsPerN, SimElapsed, PhysFrames, RunMaxV, Asleep, N,
		bTimeout ? TEXT(" (TIMEOUT)") : TEXT(""));
}

void ATowerBenchmark::EndRunAndAdvance()
{
	bRunning = false;

	// Reset in-place: destroi os atores dos cubos.
	for (UStaticMeshComponent* C : Bodies)
	{
		if (C && C->GetOwner()) { C->GetOwner()->Destroy(); }
	}
	Bodies.Reset();
	ResetCountdown = 2;

	// Avanca indices.
	Run++;
	if (Run >= RunsPerN)
	{
		Run = 0;
		NIdx++;
		if (NIdx >= NValues.Num())
		{
			Finish();
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[Torre] Avancando para N=%d"), CurrentN());
	}
}

void ATowerBenchmark::Finish()
{
	bFinished = true;
	bRunning = false;
	UE_LOG(LogTemp, Warning, TEXT("[Torre] CONCLUIDO. Resultados em %s"), *CsvPath);

	// Encerra a sessao (para PIE no editor; fecha o app num build empacotado).
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
