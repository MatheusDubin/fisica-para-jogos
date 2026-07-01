#pragma once

#include "CoreMinimal.h"

class UWorld;
class UStaticMesh;
class UStaticMeshComponent;
class UPhysicalMaterial;
class AActor;
class ACameraActor;

// Utilidades compartilhadas pelos dois cenarios (Torre e Chuva).
//
// Filosofia (identica a Unity/Godot): TUDO em codigo. Nenhuma referencia de
// nivel ou de Blueprint precisa ser ligada a mao - a cena inteira (arena,
// corpos, camera, luz) e construida via script para garantir reprodutibilidade.
//
// LEMBRETE CRITICO: Unreal trabalha em CENTIMETROS. 1 m = 100 cm. Todas as
// dimensoes aqui estao em cm. Velocidades vem em cm/s e sao convertidas para
// m/s ao gravar no CSV (para os numeros baterem 1-pra-1 com Godot/Unity).
namespace BenchmarkUtil
{
	// Aplica as configuracoes globais do benchmark:
	//  - Fixed frame rate 50 Hz => passo de fisica fixo de 0.02 s (invariante
	//    cross-engine). Com bUseFixedFrameRate o delta reportado e sempre 0.02,
	//    entao o Chaos (MaxSubstepDeltaTime=0.02, MaxSubsteps=1) avanca 0.02 por
	//    frame = 50 Hz real, equivalente ao FixedUpdate do Unity / physics_ticks
	//    do Godot. O FPS "de verdade" (wall-clock) e medido a parte na Chuva.
	//  - V-Sync desligado.
	void ApplyGlobalSettings(UWorld* World);

	// Raiz onde os CSVs sao gravados. Prioridade:
	//  1. env BENCH_OUT (absoluto).
	//  2. .../fisica-para-jogos/assignment-b/results/unreal (relativo ao projeto).
	FString ResolveOutputRoot();

	// Garante que o diretorio existe e retorna o caminho completo do arquivo.
	FString PrepareFile(const FString& SubDir, const FString& FileName);

	// Trunca o arquivo e escreve o cabecalho (primeira run da sessao).
	void WriteHeader(const FString& Path, const FString& Header);

	// Adiciona uma linha ao final do arquivo (append).
	void AppendLine(const FString& Path, const FString& Line);

	// Escreve varias linhas de uma vez (append) - usado para o debug CSV.
	void AppendLines(const FString& Path, const TArray<FString>& Lines);

	// Carrega as malhas primitivas nativas da engine.
	UStaticMesh* LoadCubeMesh();
	UStaticMesh* LoadSphereMesh();

	// Escala uniforme para que a malha tenha o half-extent alvo (em cm).
	// Robusto: deriva do bounds real da malha, entao funciona mesmo que o cubo/
	// esfera nativos da engine mudem de tamanho entre versoes.
	float UniformScaleForHalfExtent(UStaticMesh* Mesh, float TargetHalfExtentCm);

	// Cria um material fisico em runtime (Chaos usa um unico coeficiente de
	// atrito; documentamos essa diferenca vs Godot/Unity que tem static+dynamic).
	UPhysicalMaterial* MakePhysMaterial(UObject* Outer, float Friction, float Restitution,
		bool bBounceMinimum);

	// Spawna um corpo dinamico (StaticMesh + fisica simulada, massa 1 kg).
	// Retorna o componente para consultar velocidade/sleep. Loc em cm.
	UStaticMeshComponent* SpawnDynamicBody(UWorld* World, UStaticMesh* Mesh,
		UPhysicalMaterial* Mat, const FVector& Loc, float UniformScale, bool bCastShadow);

	// Cria uma parede/chao estatico. BoxSizeCm = tamanho total em cm (a malha
	// cubo nativa tem 100 cm de lado). bVisible=false => collision-only (sem
	// render), espelhando o Godot: so o chao e desenhado.
	UStaticMeshComponent* CreateWall(UWorld* World, UStaticMesh* CubeMesh,
		UPhysicalMaterial* Mat, const FVector& CenterLoc, const FVector& BoxSizeCm, bool bVisible);

	// Camera + luz direcional + skylight, com a camera olhando para LookAt.
	// Define o view target do player controller. Loc/LookAt em cm.
	// Retorna a camera para poder reposiciona-la depois (ex: reenquadrar por N).
	ACameraActor* CreateViewer(UWorld* World, const FVector& CamLoc, const FVector& LookAt);
}
