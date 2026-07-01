#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BenchmarkActorBase.generated.h"

// Componente que tica em TG_PostPhysics (depois que a fisica terminou no frame)
// e fecha a medicao do passo de fisica, chamando o dono.
UCLASS()
class UBenchmarkPostPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBenchmarkPostPhysicsComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
};

// Base para os atores de benchmark. Mede o Physics Step Time cercando a regiao
// de fisica do frame:
//   - o proprio ator tica em TG_PrePhysics  -> carimba o inicio (StepStartCycles)
//   - um componente tica em TG_PostPhysics   -> calcula o tempo decorrido (ms) e
//     chama PostPhysicsStep(), onde vai TODA a logica por passo (sleep check,
//     coleta, CSV). Como o game thread bloqueia esperando a fisica terminar
//     antes de TG_PostPhysics, esse delta e o custo real do passo de fisica -
//     o equivalente ao Stopwatch(Physics.Simulate) do Unity e ao
//     TIME_PHYSICS_PROCESS do Godot. Nunca retorna 0 (validar no primeiro run).
UCLASS(Abstract)
class ABenchmarkActorBase : public AActor
{
	GENERATED_BODY()

public:
	ABenchmarkActorBase();

	virtual void Tick(float DeltaSeconds) override;

	// Chamado uma vez por passo de fisica, DEPOIS da fisica do frame.
	// StepMs = custo do passo de fisica em ms. Subclasses implementam.
	virtual void PostPhysicsStep(float StepMs, float DeltaSeconds) {}

	// Acessado pelo componente de post-physics.
	uint64 StepStartCycles = 0;

protected:
	UPROPERTY()
	UBenchmarkPostPhysicsComponent* PostPhysicsComp = nullptr;
};
