#pragma once

#include "CoreMinimal.h"
#include "BenchmarkActorBase.h"
#include "RainBenchmark.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UPhysicalMaterial;

// Cenario 2 - A Chuva (Unreal / Chaos), in-place, sem reload de nivel.
// -----------------------------------------------------------------------------
// Ambiente (camera, luz, caixa fechada 35x80x35 m) criado UMA VEZ no BeginPlay.
// Para cada variacao (1000/5000/10000) e cada run:
//   1. Spawna N esferas num grid 3D com jitter +-3 cm (seed = 0xBEEF + run).
//   2. Aquecimento de WARMUP_STEPS passos (~5 s SIMULADOS) - descartado. As
//      esferas caem e comecam a empilhar antes de medir (o estresse de colisao
//      real e a pilha densa, nao a queda livre).
//   3. Janela de WINDOW_STEPS passos (10 s SIMULADOS): a cada passo registra o
//      Physics Step Time (medido pela base via TG_Pre/PostPhysics) e o FPS real
//      (1 / delta wall-clock, NAO o delta fixo reportado).
//      Warmup/janela sao em TEMPO DE SIMULACAO (contagem de passos), nao
//      wall-clock: com fisica acoplada N alto roda abaixo do tempo real, entao
//      10s de wall-clock mediriam so ~2s simulados (queda livre) em 10k.
//   4. Calcula media/max do step e media do FPS, grava no CSV.
//   5. Reset in-place (destroi esferas, espera frames) e proxima run.
//
// amostras = numero de passos de fisica na janela de 10 s wall-clock.
//
// NOTA DE ARQUITETURA (documentada em NOTES.md): rodamos a fisica ACOPLADA ao
// frame (sem Async Physics Tick) para poder medir o custo do passo no game
// thread. Consequencia: FPS e amostras andam juntos (ao contrario de Unity/Godot
// onde render e fisica sao desacoplados). O Physics Step Time (metrica primaria)
// permanece diretamente comparavel.
UCLASS()
class ARainBenchmark : public ABenchmarkActorBase
{
	GENERATED_BODY()

public:
	ARainBenchmark();

	virtual void BeginPlay() override;
	virtual void PostPhysicsStep(float StepMs, float DeltaSeconds) override;

private:
	// Escala em cm.
	static constexpr float SPHERE_RADIUS = 50.0f;  // esfera raio 50 cm (= 0.5 m)
	static constexpr float SPACING = 115.0f;       // > diametro (100), evita overlap
	static constexpr float SPAWN_Z = 2500.0f;      // altura de spawn acima do chao
	// Janela medida em TEMPO DE SIMULACAO (contagem de passos de fisica), NAO em
	// wall-clock. Com fisica acoplada, N alto roda abaixo do tempo real (10k a
	// ~10 FPS), entao 10s de wall-clock dariam so ~2s simulados = esferas ainda
	// em queda livre. Contando passos, todo run/engine mede o MESMO cenario
	// fisico (queda + pilha) na MESMA duracao simulada. Valores em UBenchmarkSettings
	// (WarmupSteps/WindowSteps) para ajustar sem recompilar. A janela sim-time e o
	// que corrige o bug de queda-livre; o warmup fica CURTO (~0,5s, igual G/U) de
	// proposito - warmup longo mediria pilha ja assentada (subestima).

	// Config.
	TArray<int32> Variations;
	int32 RunsPerVar = 10;
	FString Subdir = TEXT("chuva-default");
	bool bDrawWalls = false;
	int32 WarmupSteps = 25;    // ~0,5 s simulados (resolvido de UBenchmarkSettings)
	int32 WindowSteps = 500;   // 10 s simulados (resolvido de UBenchmarkSettings)

	// Recursos.
	UPROPERTY() UStaticMesh* SphereMesh = nullptr;
	UPROPERTY() UPhysicalMaterial* Mat = nullptr;
	float SphereScale = 1.0f;

	UPROPERTY() TArray<UStaticMeshComponent*> Spheres;

	// Progresso.
	int32 VarIdx = 0;
	int32 Run = 0;
	bool bRunning = false;
	bool bFinished = false;
	int32 ResetCountdown = 0;

	// Estado da coleta.
	bool bWarming = false;
	bool bCollecting = false;
	int32 StepsSinceSpawn = 0; // passos de fisica desde o spawn (dirige warmup/janela)
	double tStart = 0.0;   // wall-clock (segundos) do inicio da run (so p/ log)
	double tCollect = 0.0; // wall-clock do inicio da coleta (so p/ log de duracao real)
	double tLastFrame = 0.0;
	TArray<float> StepSamples;
	TArray<float> FpsSamples;

	FString CsvPath;

	int32 CurrentVar() const { return Variations.IsValidIndex(VarIdx) ? Variations[VarIdx] : 0; }

	void BuildBox();
	void StartRun();
	void SpawnSpheres();
	void FinalizeRun();
	void Finish();
};
