#include "BenchmarkActorBase.h"
#include "HAL/PlatformTime.h"

UBenchmarkPostPhysicsComponent::UBenchmarkPostPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// Tica depois que a fisica do frame ja terminou.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UBenchmarkPostPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ABenchmarkActorBase* Owner = Cast<ABenchmarkActorBase>(GetOwner());
	if (!Owner)
	{
		return;
	}

	const uint64 Now = FPlatformTime::Cycles64();
	float StepMs = 0.0f;
	if (Owner->StepStartCycles != 0 && Now > Owner->StepStartCycles)
	{
		StepMs = (float)(FPlatformTime::ToSeconds64(Now - Owner->StepStartCycles) * 1000.0);
	}
	Owner->PostPhysicsStep(StepMs, DeltaTime);
}

ABenchmarkActorBase::ABenchmarkActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// O ator carimba o inicio do passo de fisica ANTES da fisica do frame.
	PrimaryActorTick.TickGroup = TG_PrePhysics;

	// Precisa de um root para virar um ator posicionavel/ticavel valido.
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PostPhysicsComp = CreateDefaultSubobject<UBenchmarkPostPhysicsComponent>(TEXT("PostPhysicsTimer"));
}

void ABenchmarkActorBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Inicio da janela de medicao do passo de fisica.
	StepStartCycles = FPlatformTime::Cycles64();
}
