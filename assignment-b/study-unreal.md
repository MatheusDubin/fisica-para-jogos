# Estudo Genérico — Benchmark em Unreal Engine (Assignment B)

> Fase pré-pesquisa: entender a abordagem conceitual antes de ir à documentação.
> Perguntas abertas ao final de cada seção guiam a fase de pesquisa.

---

## 1. Configuração Base

### Fixed Timestep
No Unreal Engine, o passo de simulação física é controlado por `Fixed Framerate` e pelo `Physics Substepping`. Para 50 Hz (0.02s):

- `Project Settings > Engine > General Settings > Fixed Frame Rate = 50` (limita o jogo a 50 FPS, alinhando física e render)
- Ou via **Physics Substepping**: `Project Settings > Physics > Framerate > Max Substep Delta Time = 0.02` com `Max Substeps = 1`

A diferença é relevante: o Unreal não tem um "FixedUpdate" equivalente ao Unity — a física pode substepar dentro de um frame de render. Para benchmark controlado, é importante travar o comportamento.

### Async Physics Tick (UE 5.4+)
O Unreal introduziu o **Async Physics Tick** no UE 5.4: um tick de física com timestep fixo que roda em thread separada, desacoplado do render. Isso é mais próximo do `FixedUpdate` do Unity e do Godot. Para o benchmark, ativar isso garante maior consistência:

`bAsyncPhysicsTickEnabled = true` no `DefaultEngine.ini` ou via `Project Settings > Physics > Async Physics`.

### Primitivas a usar
- `StaticMesh` com `UCubeStaticMesh` (cubo nativo) + `UBoxComponent` como colisor
- Ou `AStaticMeshActor` com shape Box
- Ativar `Simulate Physics = true` no componente de mesh ou no `PrimitiveComponent`
- Definir massa e atrito via `UPhysicalMaterial`

---

## 2. Cenário 1 — A Torre

### Abordagem de implementação

No Unreal, instanciar atores via Blueprint ou C++:

**Blueprint:**
```
Spawn Actor From Class → BP_PhysicsBox
Location: (0, 0, i * BoxHeight)
```

**C++:**
```cpp
FActorSpawnParameters Params;
GetWorld()->SpawnActor<AStaticMeshActor>(BoxClass, FVector(0, 0, i * BoxHeight), FRotator::ZeroRotator, Params);
```

### Detecção de repouso (sleep state)

O Chaos Physics usa o conceito de **sleep** similar ao PhysX. Um corpo entra em sleep quando sua energia cinética cai abaixo de um threshold.

Em C++: `UPrimitiveComponent::IsAnySimulatingPhysics()` + velocidade via `GetPhysicsLinearVelocity()`.

Em Blueprint: nó `Get Physics Linear Velocity` + comparação com threshold.

Não há um callback `OnSleep` nativo óbvio no Chaos como no PhysX — pode ser necessário polling por frame ou registrar no `OnComponentSleep` se disponível.

### Perguntas abertas para pesquisa
- [ ] O Chaos Physics tem um evento/delegate `OnSleep` equivalente ao PhysX? Ou é necessário polling?
- [ ] Como acessar `IsBodySleeping()` por componente no Chaos em C++ e Blueprint?
- [ ] Qual o threshold de sleep padrão no Chaos e como configurá-lo via `UPhysicsSettings`?
- [ ] Como garantir que todos os atores foram instanciados antes de iniciar o timer (SpawnActor é síncrono?)?

---

## 3. Cenário 2 — Chuva de Corpos

### Abordagem de implementação

Spawnar N atores em posições aleatórias dentro de um funil (BSP ou Static Mesh como colisor estático):

```cpp
// C++
for (int i = 0; i < Count; i++) {
    FVector Pos = FMath::RandPointInBox(FunnelBounds);
    GetWorld()->SpawnActor<AStaticMeshActor>(BoxClass, Pos, FRotator::ZeroRotator);
}
```

Para 10.000 objetos, `SpawnActor` em loop pode ser lento. Alternativas:
- **Instanced Static Mesh (ISM):** renderização em batch, mas cada instância pode não ter física independente — verificar compatibilidade com Chaos.
- **Hierarchical ISM (HISM):** idem, com LOD.
- **Actor pooling:** pre-spawnar e ativar/desativar para evitar overhead de criação.

### O que medir

**Physics Step Time:** o Unreal expõe isso via console e Profiler.

**Console (em runtime):**
```
stat physics       → exibe PhysicsTime, SolverTime, etc.
stat unit          → exibe frame time breakdown (Game, Draw, GPU, Physics)
stat game          → breakdown do Game Thread
```

**API em C++:**
- `FChaosEngineInterface` / `IPhysicsInterface` — possível acesso ao tempo de simulação
- `FPhysicsCommand::ExecuteRead()` para acessar dados do solver
- `UEngine::GetStatValueForProvider()` — acesso programático às stats

**Blueprint:**
- `Get World Delta Seconds` — não é o physics step time, mas o frame delta
- Não há nó Blueprint direto para Physics Step Time — necessário C++ ou Plugin

### Automação das 10 execuções

O Unreal oferece:
- **Automation Framework:** sistema de testes automatizados (`FAutomationTest`) — mais adequado para CI, mas pode ser adaptado para benchmark
- **Blueprint com Timer:** `SetTimer` para agendar coleta e reset de cena
- **Sequencer:** gravar uma sequência de spawn e playback — menos flexível para iterações

O mais prático: um `AGameModeBase` customizado que gerencia o loop de N execuções, exporta CSV via `FFileHelper::SaveStringToFile`.

### Perguntas abertas para pesquisa
- [ ] O comando `stat physics` expõe `PhysicsTime` como valor numérico acessível via código, ou apenas como texto no overlay?
- [ ] `FPhysScene_Chaos` tem métodos públicos para consultar o tempo de simulação da última step?
- [ ] Instanced Static Mesh com Chaos Physics funciona para física independente por instância (10.000 corpos)?
- [ ] Como rodar o Unreal em modo headless (server mode sem render) para benchmarks limpos?
- [ ] Existe uma API de "Physics Stats" acessível por Blueprint, sem precisar de C++?

---

## 4. Ferramentas de Profiling Disponíveis (genérico)

| Ferramenta | Tipo | Granularidade | Disponível em build? |
|---|---|---|---|
| `stat physics` (console) | Console overlay | Média (PhysicsTime, SolverTime) | Development build |
| `stat unit` (console) | Console overlay | Baixa (frame total) | Development build |
| Unreal Insights | GUI (standalone) | Muito alta (trace por frame) | Sim (com trace ativado) |
| `FPhysicsInterface` / Chaos internal | C++ API | Alta | Sim |
| Blueprint `Get World Delta Seconds` | Blueprint | Nenhuma (frame, não physics) | Sim |

### Marcadores relevantes no Unreal Insights (a confirmar)
- `Physics` — tempo total do physics tick
- `FPhysScene_Chaos::Tick` — tick principal do Chaos
- `FPBDRigidsSolver::AdvanceOneTimeStep` — step do solver
- `ChaosPhysics` — categoria geral

---

## 5. Armadilhas Conhecidas

- **SpawnActor overhead:** instanciar 10.000 atores em um frame pode causar spike enorme e distorcer os dados. Considerar spawnar em batches por frame e começar a medir depois.
- **Garbage Collection:** o Unreal tem GC próprio (não .NET). GC stalls podem distorcer medições — verificar frequência e se há como forçar GC antes de iniciar a coleta.
- **Large World Coordinates (LWC):** dupla precisão tem custo. Posicionar a cena perto da origem (0,0,0) elimina esse overhead nas comparações.
- **Nanite / Lumen:** desativar ambos no projeto de benchmark — são features de render que não devem interferir, mas adicionam custo de frame que polui FPS.
- **`stat physics` vs Unreal Insights:** o `stat` overlay dá valores médios aproximados; o Unreal Insights dá dados per-frame precisos — preferir Insights para coleta rigorosa.

---

## 6. Perguntas Abertas Gerais (para pesquisa)

- [ ] Qual a forma recomendada pela Epic para benchmark de física com Chaos? Existe documentação específica?
- [ ] O Async Physics Tick (UE 5.4+) muda significativamente os números de PhysicsTime? Vale testá-lo como variação?
- [ ] Como configurar `Solver Iterations` (position + velocity) no Chaos de forma padronizada para comparação justa?
- [ ] Existe um modo "minimal" de projeto Unreal (sem renderização, sem editor) para benchmarks de física pura?
- [ ] O Chaos tem degradação não-linear com número de corpos (ilhas de simulação afetam isso)?
