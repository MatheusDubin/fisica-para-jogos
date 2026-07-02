#pragma once

#include "CoreMinimal.h"
#include "BenchmarkActorBase.h"
#include "TowerBenchmark.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UPhysicalMaterial;

// Cenario 1 - A Torre (Unreal / Chaos)
// -----------------------------------------------------------------------------
// 1. Constroi uma arena fechada (chao 60x60 m + 4 paredes + teto, altura 150 m).
//    Em cm: 6000 x 6000, altura 15000, espessura 200.
// 2. Empilha N cubos perfeitamente (faces tocando, spacing 100 cm = 1 m).
// 3. A cada passo de fisica conta quantos corpos dormiram (RigidBodyIsAwake==false).
// 4. Quando todos dormem (ou timeout 60 s) grava tempo + metricas e faz reset
//    in-place (destroi cubos, espera alguns frames, respawn).
// 5. Repete para cada N da lista e encerra.
//
// Tempo ate sleep = phys_frames * 0.02 (tempo de SIMULACAO, deterministico -
// identico a Unity/Godot). Velocidades e alturas sao convertidas de cm para m
// no CSV para bater 1-pra-1 com as outras engines.
UCLASS()
class ATowerBenchmark : public ABenchmarkActorBase
{
	GENERATED_BODY()

public:
	ATowerBenchmark();

	virtual void BeginPlay() override;
	virtual void PostPhysicsStep(float StepMs, float DeltaSeconds) override;

private:
	// Escala em cm.
	static constexpr float CUBE_HALF = 50.0f;    // cubo de 100 cm => half-extent 50
	static constexpr float SPACING = 100.0f;     // faces tocando, sem gap
	static constexpr float OFFSET_INICIAL = 50.0f; // centro do cubo base em z=50 (base toca o chao)
	static constexpr float TIMEOUT_S = 60.0f;
	// Distancia horizontal da camera POR EIXO (X e Y) = fator * altura da torre.
	// Com azimute 45 deg a distancia diagonal fica ~1.15*H, enquadrando a torre
	// inteira em ~80% da altura do frame (FOV vertical ~60). MENOR = mais perto.
	static constexpr float CAM_DIST_FACTOR = 0.81f;

	// Config (resolvida em BeginPlay a partir de UBenchmarkSettings).
	TArray<int32> NValues;
	int32 RunsPerN = 10;
	FString Subdir = TEXT("torre-N100-arena");
	bool bDrawWalls = false;

	// Recursos compartilhados.
	UPROPERTY() UStaticMesh* CubeMesh = nullptr;
	UPROPERTY() UPhysicalMaterial* Mat = nullptr;
	UPROPERTY() class ACameraActor* Cam = nullptr;
	float CubeScale = 1.0f;

	// Corpos da run atual.
	UPROPERTY() TArray<UStaticMeshComponent*> Bodies;

	// Estado de progresso.
	int32 NIdx = 0;
	int32 Run = 0;
	bool bRunning = false;
	bool bFinished = false;
	int32 ResetCountdown = 0;

	// Metricas da run atual.
	float SimElapsed = 0.0f;
	int32 PhysFrames = 0;
	float RunMaxV = 0.0f;      // em m/s
	float tFirstSleep = -1.0f;
	float tHalfSleep = -1.0f;
	TArray<FString> DebugBuffer;

	FString CsvPath;
	FString DebugPath;

	int32 CurrentN() const { return NValues.IsValidIndex(NIdx) ? NValues[NIdx] : 0; }

	void BuildArena();
	void FrameTowerCamera(int32 N);
	void StartRun();
	void RecordRun(bool bTimeout, int32 Asleep);
	void EndRunAndAdvance();
	void Finish();
};
