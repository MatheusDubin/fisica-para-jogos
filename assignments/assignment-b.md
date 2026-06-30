# Assignment B — Physics Benchmark Prático (Grau B)

## Entregáveis
1. **Repositório de Código** (GitHub/GitLab) — cenas para as três engines
2. **Documento de Resultados** (`results.md` no repo) — tabelas com tempos brutos, desvio padrão e médias finais
3. **Apresentação de Slides** — vídeos curtos, gráficos comparativos, considerações críticas

---

## O que o Grau A nos diz sobre o Grau B

O Grau A levantou as arquiteturas internas de cada engine. Esses achados informam diretamente o que esperamos observar nos benchmarks e como interpretar os dados coletados.

### Cenário 1 — A Torre

| Engine | Solver (do Grau A) | Implicação para a Torre |
|---|---|---|
| **Unity** | PhysX 4.x — Gauss-Seidel iterativo, warm starting ativo | Solver iterativo acumula erro em pilhas altas; warm starting ajuda convergência entre frames, mas pilhas de 100+ blocos podem apresentar jitter residual |
| **Unreal** | Chaos — XPBD (Extended Position-Based Dynamics) + **Shock Propagation** | Shock Propagation distribui impulsos da base ao topo explicitamente — feature projetada para exatamente este cenário; verificar se está ativo e documentar |
| **Godot** | Jolt — impulse-based, multi-threaded | Solver diferente dos outros dois; multi-threading beneficia pouco a Torre (cenário predominantemente estático); comparar estabilidade com Unity (mesmo paradigma impulse-based) |

### Cenário 2 — Chuva de Corpos

| Engine | Arquitetura relevante (do Grau A) | Implicação para a Chuva |
|---|---|---|
| **Unity** | PhysX path (GameObject): SAP/MBP broadphase, worker thread dedicada, threading parcial | Broadphase é o provável gargalo a partir de 5k objetos; FPS cai junto com Physics Step Time pois compartilham o sync point da main thread |
| **Unreal** | Chaos: islands em paralelo, **Async Physics Tick** (UE 5.4+), double precision (LWC) | FPS e Physics Step Time são **desacoplados** pelo Async Physics Tick — FPS pode permanecer alto mesmo quando Step Time excede 20ms; LWC eleva o custo base por objeto |
| **Godot** | Jolt: job system nativo, todos os cores | Melhor escalabilidade esperada das três para variações altas (5k/10k); TIME_PHYSICS_PROCESS inclui overhead de scheduling do job system |

> Estes achados não são hipóteses a confirmar antes dos dados — são o contexto que usaremos para **explicar** os dados depois de coletados, e para responder "os resultados confirmam o Grau A?" nos slides finais.

---

## Enunciado Original

### Contexto
A partir do levantamento arquitetural do Grau A, colocar a teoria à prova. Motores diferentes possuem algoritmos de detecção de colisão e solvers distintos. O objetivo é medir os reais limites e gargalos via testes de estresse (benchmarks).

### Objetivo
Desenvolver benchmarks comparando Unity, Unreal Engine e Godot em cenários idênticos de simulação de corpos rígidos, aplicando metodologia estatística para garantir rigor nos resultados.

---

## Cenários de Benchmark

### Variáveis de Controle (obrigatório manter iguais nas 3 engines)
- Usar **malhas primitivas** apenas (sem Mesh Colliders complexos)
- Mesma **massa** e **atrito** para todos os objetos
- **Fixed Timestep = 0.02s (50Hz)** em todas as engines

---

### Cenário 1: A Torre (Estabilidade e Jittering)

**Setup:** Pilha vertical com grande quantidade de corpos rígidos primitivos (caixas perfeitamente empilhadas).

**O que medir:**
- Tempo (segundos) para a pilha entrar em **repouso total (sleep state)**
- Presença de **jitter** (trepidação) excessivo
- Se a física **colapsa** ou não

**Quantidade sugerida de blocos:** testar com 50, 100, 200 (definir o limite de colapso)

---

### Cenário 2: Chuva de Corpos (Estresse de Colisão e Performance)

**Setup:** Instanciar simultaneamente uma grande quantidade de corpos primitivos em um espaço contido (funil gigante).

**Variações obrigatórias:**
| Variação | Quantidade de objetos |
|---|---|
| V1 | 1.000 |
| V2 | 5.000 |
| V3 | 10.000 |

**O que medir:**
- **Physics Step Time** médio (em ms ou ns)
- **FPS** da simulação sob estresse

---

## Metodologia de Coleta de Dados

Para cada variação de cenário, em cada engine:

1. Realizar **10 execuções isoladas**, registrando o tempo médio de processamento da física
2. Calcular **média simples** e **variância** → desvio padrão σ
3. **Descartar** execuções fora do intervalo `[Média ± σ]`
4. Calcular **Média Final** apenas com os valores dentro da faixa

### Template de Tabela de Resultados

```
Engine: Unity | Cenário: 2 | Variação: 1.000 objetos
| Execução | Physics Step Time (ms) | FPS | Dentro da faixa? |
|---|---|---|---|
| 1 | ... | ... | ✅ / ❌ |
...
| Média Bruta | ... | ... | |
| Desvio Padrão | ... | ... | |
| Média Final | ... | ... | |
```

---

## Plano de Execução

### Fase 1 — Setup das Engines e Projetos Base
- [ ] Criar projeto Unity (versão LTS mais recente)
- [ ] Criar projeto Unreal Engine (UE5 LTS)
- [ ] Criar projeto Godot (4.x stable)
- [ ] Configurar Fixed Timestep = 0.02s em todas
- [ ] Criar repositório no GitHub com subpastas `unity/`, `unreal/`, `godot/`

### Fase 2 — Implementação Cenário 1 (Torre)
- [ ] **Unity:** script C# para spawnar pilha + detectar sleep state via `Rigidbody.IsSleeping()`
- [ ] **Unreal:** Blueprint/C++ para pilha + detecção via `IsSimulatingPhysics` + sleep callback
- [ ] **Godot:** GDScript para pilha + detecção via `sleeping` property do RigidBody3D
- [ ] Automatizar 10 execuções e exportar CSV com tempos

### Fase 3 — Implementação Cenário 2 (Chuva)
- [ ] **Unity:** script C# com Profiler API (`Profiler.GetCounter`) para capturar Physics Step Time
- [ ] **Unreal:** usar `stat unit` / `ProfileGPU` ou Blueprint para capturar `PhysicsTime`
- [ ] **Godot:** usar `Performance.get_monitor(Performance.PHYSICS_PROCESS_TIME)` + FPS counter
- [ ] Automatizar variações 1k / 5k / 10k e exportar CSV

### Fase 4 — Coleta e Análise Estatística
- [ ] Rodar 10 execuções por variação por engine (total: 3 engines × 3 variações × 10 runs = 90 medições para Cenário 2)
- [ ] Calcular média, variância, desvio padrão
- [ ] Filtrar outliers e calcular média final
- [ ] Criar `results.md` com todas as tabelas

### Fase 5 — Visualização e Slides
- [ ] Gravar vídeos curtos de cada teste (mostrando onde a física "quebra")
- [ ] Gerar gráficos de barras comparando tempos finais do Cenário 2
- [ ] Responder considerações críticas:
  - Qual engine suportou mais corpos rígidos?
  - Qual entregou a pilha mais estável?
  - Os resultados confirmam o Grau A?
- [ ] Montar slides com vídeos + gráficos + análise crítica

---

## Dicas Técnicas por Engine

> All values below implement the spec in `SCENE_SPEC.md`. Apply every setting before running any benchmark run.

### Unity

**Scene spec implementation**
- Gravity: `Physics.gravity = new Vector3(0, -9.81f, 0);` (default — confirm it hasn't been changed in Project Settings → Physics)
- Fixed timestep: Project Settings → Time → Fixed Timestep = `0.02`
- Sleep thresholds: `Physics.sleepThreshold = 0.05f;` (this controls the velocity magnitude; Unity merges linear+angular into one scalar — set to `0.05`)
- Box collider: Add `BoxCollider` with Size `(1, 1, 1)` on a GameObject scaled `(1, 1, 1)`
- Rigidbody: Mass = `1`, Drag = `0`, Angular Drag = `0.05` (engine default — do not change)
- Friction & bounciness: Create a `PhysicMaterial` with Static Friction `0.6`, Dynamic Friction `0.4`, Bounciness `0.0`, set on the collider
- Tower spawn: `position = new Vector3(0, n - 0.5f, 0)` for box index `n` starting at 1
- Rain spawn: random XZ in `[-17.5, 17.5]`, Y = `22 + Random.Range(0, 5)`

**Profiling**
- `using UnityEngine.Profiling;` + `Profiler.GetCounter("Physics.Processing")` for Physics Step Time
- Sleep detection (Scenario 1): `Rigidbody.IsSleeping()` — poll every `FixedUpdate`, start 1 s countdown when all return true

---

### Unreal Engine

**Unit conversion reminder:** all meter values × 100 = centimeters; Up axis = Z (not Y).

**Scene spec implementation**
- Gravity: Project Settings → Physics → `Z = -981 cm/s²` (default — verify)
- Fixed framerate: Project Settings → Engine → Fixed Frame Rate = `50` (or `bSmoothFrameRate = false`, `FixedFrameRate = 50.0` in `DefaultEngine.ini`)
- Sleep threshold: Project Settings → Physics → `SleepThresholdMultiplier` — this multiplies the default; to get ~0.05 m/s set it to `1.0` and confirm `DefaultSleepLinearVelocityThreshold = 0.05` in `DefaultEngine.ini` under `[/Script/Engine.PhysicsSettings]`
- Box collider: Static Mesh with Box Collision, scale `(1, 1, 1)` at 100 cm default cube
- Physics material: Create `PhysicalMaterial` asset, set Friction `0.6`, Restitution `0.0`; Unreal uses one friction value — use `0.6` for both static and dynamic (or enable `bOverrideFrictionCombineMode`)
- Mass: On the `StaticMeshComponent` → Physics → Mass = `1 kg` (override if auto-calculate gives a different value)
- Tower spawn (Z-up): first box center at `Z = 50 cm`, each subsequent box at `Z = N × 100 cm - 50 cm`
- Rain spawn: random XY in `[-1750, 1750] cm`, Z = `2200 + FMath::RandRange(0, 500) cm`

**Profiling**
- `stat physics` in the console — shows `PhysicsTime` per frame
- Async Physics Tick (UE 5.4+): if enabled, FPS and PhysicsTime are decoupled — note this in results
- Sleep detection (Scenario 1): bind to `OnComponentSleep` event on each mesh component; start 1 s countdown when all have fired

---

### Godot

**Scene spec implementation**
- Gravity: Project Settings → Physics → 3D → `Default Gravity = 9.81`, `Default Gravity Vector = (0, -1, 0)` (default — confirm)
- Fixed timestep: Project Settings → Physics → Common → `physics/common/physics_ticks_per_second = 50`
- Sleep thresholds: Project Settings → Physics → 3D → `sleep_threshold_linear = 0.05`, `sleep_threshold_angular = 0.05`
- Box shape: `BoxShape3D` with size `Vector3(1, 1, 1)` on a `RigidBody3D`
- Mass: `RigidBody3D.mass = 1.0`
- Physics material: `PhysicsMaterial` with Friction `0.6`, Rough = false, Bounce `0.0`; assign to `CollisionShape3D`
- Tower spawn: `position = Vector3(0, n - 0.5, 0)` for box index `n` starting at 1
- Rain spawn: `position = Vector3(randf_range(-17.5, 17.5), 22.0 + randf_range(0, 5), randf_range(-17.5, 17.5))`

**Profiling**
- Physics Step Time: `Performance.get_monitor(Performance.PHYSICS_PROCESS_TIME)` — returns microseconds, divide by 1000 for ms
- FPS: `Engine.get_frames_per_second()`
- Sleep detection (Scenario 1): `RigidBody3D.sleeping` property — poll in `_physics_process`, start 1 s countdown when all are `true`

---

## Estrutura do Repositório (Sugerida)

```
project-f-benchmark/
├── README.md
├── results.md                   ← tabelas com todos os dados
├── unity/
│   ├── Assets/Scenes/
│   │   ├── Scenario1_Tower.unity
│   │   └── Scenario2_Rain.unity
│   └── Assets/Scripts/
│       ├── TowerBenchmark.cs
│       └── RainBenchmark.cs
├── unreal/
│   ├── Source/
│   └── Content/Maps/
├── godot/
│   ├── scenes/
│   └── scripts/
└── data/
    ├── raw/                     ← CSVs brutos das 10 execuções
    └── processed/               ← médias finais e desvio padrão
```

---

## Considerações Críticas (a responder nos slides)

- Qual engine suportou mais corpos rígidos sem colapso?
- Qual entregou a pilha mais estável (menos jitter)?
- Os resultados do benchmark confirmam o que foi estudado no Grau A?
- Houve diferença significativa entre os solvers (Chaos vs PhysX/Havok vs Jolt)?

---

## Dependência
> Este trabalho depende dos resultados do **Assignment A** para contextualizar as análises.
> Ver: `assignments/assignment-a.md`

---

## Status
- [ ] Fase 1 — Setup das engines e repositório
- [ ] Fase 2 — Cenário 1 implementado nas 3 engines
- [ ] Fase 3 — Cenário 2 implementado nas 3 engines
- [ ] Fase 4 — Coleta de dados e análise estatística
- [ ] Fase 5 — Visualização e slides
- [ ] Entrega no Moodle + repositório público
