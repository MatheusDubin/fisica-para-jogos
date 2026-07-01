#include "BenchmarkCommon.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "UObject/ConstructorHelpers.h"

namespace BenchmarkUtil
{
	void ApplyGlobalSettings(UWorld* World)
	{
		if (GEngine)
		{
			// Passo de fisica fixo em 50 Hz (0.02 s) - invariante cross-engine.
			GEngine->bUseFixedFrameRate = true;
			GEngine->FixedFrameRate = 50.0;

			if (World)
			{
				GEngine->Exec(World, TEXT("r.VSync 0"));
			}
		}
	}

	FString ResolveOutputRoot()
	{
		const FString Env = FPlatformMisc::GetEnvironmentVariable(TEXT("BENCH_OUT"));
		if (!Env.IsEmpty())
		{
			return Env;
		}

		// ProjectDir = .../fisica-para-jogos/Projects/physics_unreal/
		// Dois niveis acima = .../fisica-para-jogos/
		const FString Root = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::ProjectDir(), TEXT("../../assignment-b/results/unreal")));
		return Root;
	}

	FString PrepareFile(const FString& SubDir, const FString& FileName)
	{
		const FString Dir = FPaths::Combine(ResolveOutputRoot(), SubDir);
		IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);
		return FPaths::Combine(Dir, FileName);
	}

	void WriteHeader(const FString& Path, const FString& Header)
	{
		FFileHelper::SaveStringToFile(Header + TEXT("\n"), *Path,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	void AppendLine(const FString& Path, const FString& Line)
	{
		FFileHelper::SaveStringToFile(Line + TEXT("\n"), *Path,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
			&IFileManager::Get(), FILEWRITE_Append);
	}

	void AppendLines(const FString& Path, const TArray<FString>& Lines)
	{
		if (Lines.Num() == 0)
		{
			return;
		}
		FString Blob;
		Blob.Reserve(Lines.Num() * 64);
		for (const FString& L : Lines)
		{
			Blob += L;
			Blob += TEXT("\n");
		}
		FFileHelper::SaveStringToFile(Blob, *Path,
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
			&IFileManager::Get(), FILEWRITE_Append);
	}

	UStaticMesh* LoadCubeMesh()
	{
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	UStaticMesh* LoadSphereMesh()
	{
		return LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	}

	float UniformScaleForHalfExtent(UStaticMesh* Mesh, float TargetHalfExtentCm)
	{
		if (!Mesh)
		{
			return 1.0f;
		}
		const float HalfExtent = Mesh->GetBounds().BoxExtent.X;
		return HalfExtent > KINDA_SMALL_NUMBER ? (TargetHalfExtentCm / HalfExtent) : 1.0f;
	}

	UPhysicalMaterial* MakePhysMaterial(UObject* Outer, float Friction, float Restitution,
		bool bBounceMinimum)
	{
		UPhysicalMaterial* Mat = NewObject<UPhysicalMaterial>(Outer ? Outer : (UObject*)GetTransientPackage());
		Mat->Friction = Friction;
		Mat->Restitution = Restitution;
		Mat->bOverrideFrictionCombineMode = true;
		Mat->FrictionCombineMode = EFrictionCombineMode::Average;
		Mat->bOverrideRestitutionCombineMode = true;
		// Torre: bounce combine Minimum (energia minima). Chuva: Average.
		Mat->RestitutionCombineMode = bBounceMinimum ? EFrictionCombineMode::Min : EFrictionCombineMode::Average;
		return Mat;
	}

	UStaticMeshComponent* SpawnDynamicBody(UWorld* World, UStaticMesh* Mesh,
		UPhysicalMaterial* Mat, const FVector& Loc, float UniformScale, bool bCastShadow)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(Loc), Params);
		if (!Actor)
		{
			return nullptr;
		}

		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Actor);
		Actor->SetRootComponent(Comp);
		Comp->SetStaticMesh(Mesh);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetRelativeScale3D(FVector(UniformScale));
		// Posiciona ANTES de registrar: um AActor "cru" nao tem root ao ser
		// spawnado, entao o transform do SpawnActor se perde ao definir o root
		// depois. Setar a location relativa (o comp e o root, sem pai => relativa
		// = mundo) garante o spawn no lugar certo.
		Comp->SetRelativeLocation(Loc);
		Comp->SetCollisionProfileName(TEXT("PhysicsActor"));
		Comp->SetCastShadow(bCastShadow);
		Comp->RegisterComponent();

		Comp->SetPhysMaterialOverride(Mat);
		Comp->SetSimulatePhysics(true);
		Comp->SetMassOverrideInKg(NAME_None, 1.0f, /*bOverrideMass=*/true);
		Comp->WakeRigidBody();
		return Comp;
	}

	UStaticMeshComponent* CreateWall(UWorld* World, UStaticMesh* CubeMesh,
		UPhysicalMaterial* Mat, const FVector& CenterLoc, const FVector& BoxSizeCm, bool bVisible)
	{
		if (!World || !CubeMesh)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform(CenterLoc), Params);
		if (!Actor)
		{
			return nullptr;
		}

		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(Actor);
		Actor->SetRootComponent(Comp);
		Comp->SetStaticMesh(CubeMesh);
		Comp->SetMobility(EComponentMobility::Static);
		// Malha cubo nativa = 100 cm de lado. Escala = tamanho desejado / 100.
		Comp->SetRelativeScale3D(BoxSizeCm / 100.0f);
		// BUG CORRIGIDO: sem isto todas as paredes ficavam na origem (0,0,0),
		// empilhadas, e a caixa/arena nao continha nada. Posiciona a parede
		// ANTES de registrar (root sem pai => location relativa = mundo).
		Comp->SetRelativeLocation(CenterLoc);
		Comp->SetCollisionProfileName(TEXT("BlockAll"));
		Comp->SetCastShadow(false);
		if (!bVisible)
		{
			Comp->SetVisibility(false);
		}
		Comp->RegisterComponent();
		Comp->SetPhysMaterialOverride(Mat);
		return Comp;
	}

	ACameraActor* CreateViewer(UWorld* World, const FVector& CamLoc, const FVector& LookAt)
	{
		if (!World)
		{
			return nullptr;
		}

		// Luz direcional (Movable para nao exigir build de lighting).
		if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>())
		{
			Sun->SetMobility(EComponentMobility::Movable);
			Sun->SetActorRotation(FRotator(-50.0f, -35.0f, 0.0f));
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LC->SetIntensity(3.0f);
			}
		}

		// Skylight para iluminacao ambiente (senao os corpos ficam pretos).
		if (ASkyLight* Sky = World->SpawnActor<ASkyLight>())
		{
			Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable);
			Sky->GetLightComponent()->SetIntensity(1.0f);
		}

		// Camera + view target.
		const FRotator LookRot = (LookAt - CamLoc).Rotation();
		ACameraActor* Cam = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(),
			FTransform(LookRot, CamLoc));
		if (Cam)
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				PC->SetViewTarget(Cam);
			}
		}
		return Cam;
	}
}
