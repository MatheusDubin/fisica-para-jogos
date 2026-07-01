# How-To: Unreal Engine — Setup, Profiling e Métricas

> Guia prático para quem está saindo do zero ou está enferrujado.
> Versão de referência: UE 5.x LTS (5.3, 5.4 ou 5.5).
> Motor de física: **Chaos Physics** (padrão desde UE5 — não há como desativar).

---

## ⚠️ Lembrete Crítico: Escala em Centímetros

**Unreal trabalha em centímetros.** Tudo que é 1 metro nas outras engines é 100 cm aqui.


| Medida             | Unity / Godot  | Unreal                |
| ------------------ | -------------- | --------------------- |
| 1 cubo             | 1m × 1m × 1m   | 100cm × 100cm × 100cm |
| Esfera raio        | 0.5m           | 50cm                  |
| Gravidade          | -9.81 m/s²     | **-980 cm/s²**        |
| Pilha de 100 cubos | 100m de altura | 10.000cm de altura    |


Errar a escala invalida a comparação.

---



## 1. Criar o Projeto

1. Abrir **Epic Games Launcher**
2. **Unreal Engine > Launch**
3. Na tela de projetos: **Games > Blank**
4. Configurações:
  - Blueprint **ou** C++(Blueprint é mais rápido para começar; C++ para captura de métricas mais precisa)
  - Quality: Scalable
  - **Starter Content: desativado** (não precisamos)
5. Nomear: `ProjectFUnreal` (sem espaços)
6. Criar

---



## 2. Configurar Fixed Timestep

`Edit > Project Settings > Engine > Physics`

- **Max Substep Delta Time:** `0.02` (equivale ao Fixed Timestep de 50Hz)
- **Max Substeps:** `1`

**Alternativa (UE 5.4+) — Async Physics Tick:**
`Edit > Project Settings > Engine > Physics > Enable Async Scene`  
Ativar se disponível — é o modo mais preciso para benchmarks pois desacopla física de rendering.

---



## 3. Configurar Physics Settings — Anotar Padrões

`Edit > Project Settings > Engine > Physics`

Antes de qualquer alteração, **anotar os valores padrão**:


| Configuração                         | Valor padrão (anotar aqui) |
| ------------------------------------ | -------------------------- |
| Default Solver Iterations (Position) | ___                        |
| Default Solver Iterations (Velocity) | ___                        |
| Sleep Threshold Multiplier           | ___                        |
| Bounce Threshold Velocity            | 200                        |
| Gravity Z                            | -980 (deve ser -980)       |


> **Não alterar nenhum desses valores.**

---



## 4. Desativar V-Sync e Frame Cap

No console do editor ( `` backtick durante play):

```
r.VSync 0
t.MaxFPS 0
```

Para aplicar permanentemente, adicionar em `Config/DefaultEngine.ini`:

```ini
[/Script/Engine.RendererSettings]
r.VSync=0

[SystemSettings]
t.MaxFPS=0
```

---



## 5. Medir Physics Step Time — Via Console (método simples)

Durante play mode, pressionar  `` (backtick) para abrir o console e digitar:

```
stat physics
```

Isso exibe no HUD:

- **Physics Time:** tempo do passo de física em ms — **esta é a métrica que queremos**
- Bodies Simulated, Sleep Bodies, Active Bodies

```
stat unit
```

Exibe:

- **Frame:** tempo total do frame
- **Game:** tempo do game thread
- **Draw:** tempo de rendering
- **GPU:** tempo de GPU

Para remover: digitar o mesmo comando novamente.

---



## 6. Medir Physics Step Time — Via Unreal Insights (método preciso)

Unreal Insights é a ferramenta correta para capturar dados das 10 runs automaticamente.

### 6.1 Iniciar captura

No console durante play:

```
Trace.Start C:/benchmark/run_01.utrace
```



### 6.2 Parar captura

```
Trace.Stop
```



### 6.3 Abrir o arquivo

- Abrir **Unreal Insights** (está em `Engine/Binaries/Win64/UnrealInsights.exe`)
- File > Open Trace > selecionar o `.utrace`
- Na view **Timing Insights:**
  - Expandir a thread `Physics`
  - Selecionar a janela de tempo do teste
  - No painel inferior: ver o tempo médio do evento `FPhysScene_Chaos::EndFrame` ou `Physics`



### 6.4 Extrair o valor

- Selecionar a região do gráfico correspondente à janela de coleta (ex: primeiros 10s)
- No painel de estatísticas: anotar **Average (ms)** do evento de física

---



## 7. Detectar Sleep State (Cenário 1 — Torre)



### Via Blueprint

1. Selecionar o cubo (Static Mesh Actor com Physics habilitada)
2. No painel Details: `Physics > Simulate Physics = true`
3. No Event Graph do Actor:
  - Adicionar evento: **On Component Sleep** (disponível em Static Mesh Component)
  - Este evento dispara quando o body dorme

Para contar todos os corpos dormindo:

```
// Pseudológica em Blueprint:
Event On Component Sleep
→ Incrementar contador "dormindo"
→ If contador == NUM_CUBOS
    → Calcular tempo decorrido
    → Salvar resultado
    → Recarregar level
```



### Via C++

```cpp
// No BeginPlay do benchmark actor:
TArray<UStaticMeshComponent*> todosOsCorpos;

// Para cada corpo:
UStaticMeshComponent* mesh = GetComponentByClass<UStaticMeshComponent>();
mesh->OnComponentSleep.AddDynamic(this, &AMinhaClasse::OnCorpoDormiu);

// Callback:
void AMinhaClasse::OnCorpoDormiu(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
    CorposDormindo++;
    if (CorposDormindo >= NumCubos)
    {
        float TempoAteSleep = GetWorld()->GetTimeSeconds() - TempoInicio;
        UE_LOG(LogTemp, Warning, TEXT("Sleep em: %f segundos"), TempoAteSleep);
        SalvarResultado(TempoAteSleep);
        RecarregarLevel();
    }
}
```

---



## 8. Medir FPS



### Via Blueprint

Node: **Get Frame Rate** (retorna float em FPS)

### Via C++

```cpp
// Opção 1: via FApp
float fps = 1.0f / FApp::GetDeltaTime();

// Opção 2: via variável global do engine
extern ENGINE_API float GAverageFPS;
float fps = GAverageFPS;
```

> **Lembrar:** com Async Physics Tick ativo, o FPS **não reflete** o custo de física. Medir FPS de qualquer forma para documentar, mas Physics Time (do `stat physics` ou Insights) é a métrica principal.

---



## 9. Salvar Dados em Arquivo (C++)

```cpp
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void SalvarLinha(FString Linha)
{
    FString Caminho = FPaths::ProjectSavedDir() + TEXT("benchmark.csv");
    FFileHelper::SaveStringToFile(
        Linha + TEXT("\n"),
        *Caminho,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append  // adiciona ao final em vez de sobrescrever
    );
}

// Uso:
// SalvarLinha(FString::Printf(TEXT("%d,%.3f,%.1f"), RunAtual, PhysicsTimeMs, FPS));
```

O arquivo fica em: `ProjectFUnreal/Saved/benchmark.csv`

---



## 10. Automatizar 10 Runs (Recarregar Level)

```cpp
#include "Kismet/GameplayStatics.h"

void RecarregarLevel()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()));
}
```

Para persistir o número da run entre recarregamentos, usar uma **Game Instance** (sobrevive ao reload de level):

1. `File > New C++ Class > Game Instance` — nomear `BenchmarkGameInstance`
2. Adicionar variável: `int32 RunAtual = 0;`
3. `Edit > Project Settings > Maps & Modes > Game Instance Class` → selecionar `BenchmarkGameInstance`
4. No benchmark actor: `UBenchmarkGameInstance* gi = Cast<UBenchmarkGameInstance>(GetGameInstance());`

---



## 11. Criar os Objetos Físicos



### Cubo (Cenário 1 — Torre)

1. `Place Actors > Shapes > Cube` — ou arrastar da Content Browser
2. Scale: `(1.0, 1.0, 1.0)` — em UE isso = 100cm por lado ✓
3. No painel **Details:**
  - `Physics > Simulate Physics = true`
  - `Physics > Mass = 1.0 kg`
  - `Collision > Collision Preset = PhysicsActor`
4. Criar um **Physical Material:**
  - `Content Browser > Add > Physics > Physical Material`
  - Friction: `0.5` (Static), `0.4` (Dynamic)
  - Restitution: `0.0` ← crítico para Torre
  - Arrastar o Physical Material para o campo `Phys Material Override` do Static Mesh Component



### Esfera (Cenário 2 — Chuva)

- `Place Actors > Shapes > Sphere`
- Scale: `(0.5, 0.5, 0.5)` — resulta em esfera de raio 50cm ✓
- Mesmas configurações de Physics
- Restitution: `0.3` para Chuva



### Verificar Shock Propagation (Cenário 1)

`Edit > Project Settings > Engine > Physics > Solver`

- Procurar por **Shock Propagation** ou **Enable Shock Propagation**
- **Anotar se está ativo ou não** — não alterar
- Documentar no arquivo de resultados (impacta diretamente a Torre)

---



## 12. Checklist antes de Rodar

- [ ] Max Substep Delta Time = 0.02 confirmado
- [ ] Gravity Z = -980 confirmado
- [ ] V-Sync desativado (`r.VSync 0`)
- [ ] Frame cap removido (`t.MaxFPS 0`)
- [ ] Restitution = 0.0 no Physical Material dos cubos (Torre)
- [ ] Valores padrão do solver anotados (não alterados)
- [ ] Status do Shock Propagation documentado
- [ ] Game Instance configurada para persistir RunAtual
- [ ] CSV sendo gravado com append (não sobrescrevendo)
- [ ] Build em **Shipping** — `File > Package Project > Build Configuration = Shipping`
- [ ] Escala de todos os objetos verificada (cm, não m)

---



## 13. Referência Rápida


| O que fazer                    | Como                                                   |
| ------------------------------ | ------------------------------------------------------ |
| Ver Physics Time em tempo real | Console: `stat physics`                                |
| Capturar trace para Insights   | Console: `Trace.Start caminho.utrace`                  |
| Corpo dormiu?                  | Evento `On Component Sleep`                            |
| Tempo da simulação             | `GetWorld()->GetTimeSeconds()`                         |
| FPS                            | `GAverageFPS` ou `1.0f / FApp::GetDeltaTime()`         |
| Salvar arquivo                 | `FFileHelper::SaveStringToFile` com `FILEWRITE_Append` |
| Recarregar level               | `UGameplayStatics::OpenLevel(GetWorld(), LevelName)`   |
| Persistir dados entre runs     | `UGameInstance` subclassificada                        |
| Desativar V-Sync               | `r.VSync 0` no console                                 |
| Remover frame cap              | `t.MaxFPS 0` no console                                |


