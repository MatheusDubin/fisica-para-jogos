#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BenchmarkSettings.generated.h"

UENUM(BlueprintType)
enum class EBenchScenario : uint8
{
	Tower  UMETA(DisplayName = "Torre (Cenario 1)"),
	Rain   UMETA(DisplayName = "Chuva (Cenario 2)")
};

// Configuracao do benchmark. Aparece em Project Settings > Game > "Benchmark",
// persiste em Config/DefaultGame.ini. Tudo controlado sem tocar em Blueprint.
//
// Override por linha de comando (util para rodar em batch):
//   -bench=tower  |  -bench=rain
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Benchmark"))
class UBenchmarkSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	// Qual cenario rodar nesta sessao (rode UM por vez).
	UPROPERTY(config, EditAnywhere, Category = "Geral")
	EBenchScenario Scenario = EBenchScenario::Tower;

	// Debug: desenhar as paredes/teto (normalmente collision-only, so o chao e
	// visivel - igual a Godot/Unity - para o FPS ser comparavel). Ligue para
	// VER a arena/caixa e validar a contencao; desligue para os runs oficiais.
	UPROPERTY(config, EditAnywhere, Category = "Geral")
	bool bDrawWalls = false;

	// --- Cenario 1: Torre ---

	// Lista de N (numero de cubos). Default {100}. Pode virar sweep: {10,15,20,25,30}.
	UPROPERTY(config, EditAnywhere, Category = "Torre")
	TArray<int32> TowerN = { 100 };

	// Runs por valor de N.
	UPROPERTY(config, EditAnywhere, Category = "Torre")
	int32 TowerRunsPerN = 10;

	// Subpasta de saida em results/unreal/.
	UPROPERTY(config, EditAnywhere, Category = "Torre")
	FString TowerSubdir = TEXT("torre-N100-arena");

	// --- Cenario 2: Chuva ---

	// Variacoes de numero de esferas.
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	TArray<int32> RainVariations = { 1000, 5000, 10000 };

	// Runs por variacao.
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	int32 RainRuns = 10;

	// Subpasta de saida em results/unreal/.
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	FString RainSubdir = TEXT("chuva-default");

	// Warmup (descartado) em PASSOS de fisica. 25 = ~0,5 s simulados (igual a
	// Godot/Unity). Curto de proposito: a janela sim-time ja captura queda+pilha;
	// warmup longo mediria a pilha ja assentada (mais barata) e subestimaria.
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	int32 RainWarmupSteps = 25;

	// Janela de coleta em PASSOS de fisica. 500 = 10 s simulados.
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	int32 RainWindowSteps = 500;

	// Coluna EXPLORATORIA "Chaos-otimizado": aplica um bundle de CVars do Chaos
	// (menos iterações, sem CCD, paralelismo, etc.) para ver quanto dá para
	// reduzir o step time. Grava em subpasta separada (`chuva-optimized`), então
	// NÃO afeta a coluna default (`chuva-default`), que continua re-rodável.
	// Off = reseta as CVars aos padrões (runs default ficam limpos na mesma sessão).
	UPROPERTY(config, EditAnywhere, Category = "Chuva")
	bool bRainOptimize = false;

	// Resolve o cenario efetivo (settings + override de linha de comando).
	static EBenchScenario ResolveScenario();
};
