#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BenchmarkGameMode.generated.h"

// GameMode que dispara o benchmark automaticamente ao dar Play - equivalente ao
// autoload do Godot / [RuntimeInitializeOnLoadMethod] do Unity. Nenhum setup de
// nivel necessario: basta este GameMode estar configurado como default
// (feito no DefaultEngine.ini) e apertar Play.
UCLASS()
class ABenchmarkGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABenchmarkGameMode();

	virtual void BeginPlay() override;
};
