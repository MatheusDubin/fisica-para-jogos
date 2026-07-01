# Unreal — Notas de Metodologia e Decisões

> Diário de bordo da implementação Unreal. Espelha `results/godot/NOTES.md` e
> `results/unity/NOTES.md` para os três engines serem comparáveis 1-pra-1 no
> relatório final.

---

## 1. Ambiente

| Item | Valor |
|---|---|
| Engine | Unreal Engine **5.8** |
| Physics Engine | **Chaos Physics** (padrão desde UE5, não desativável) |
| Linguagem | **C++** (módulo `physics_unreal`) |
| Toolchain | VS 2026 Community (MSVC 14.51.36231) + Windows SDK 10.0.26100 |
| Render | Padrão do projeto (DX12/SM6) |

### Project Settings travados

| Setting | Valor | Onde |
|---|---|---|
| Substepping | On | `DefaultEngine.ini [/Script/Engine.PhysicsSettings]` |
| Max Substep Delta Time | **0.02** | idem |
| Max Substeps | **1** | idem |
| Gravity Z | **-980** cm/s² (= -9.81 m/s²) | idem (`DefaultGravityZ`) |
| V-Sync | desligado | `DefaultEngine.ini [SystemSettings] r.VSync=0` + runtime |
| Fixed Frame Rate | **50 Hz** (ligado em runtime) | `BenchmarkUtil::ApplyGlobalSettings` |
| Solver Iterations / Sleep Threshold | **PADRÃO — não alterados** | Defaults do Chaos (ver abaixo) |

### Defaults do solver do Chaos (lidos direto do código-fonte da UE 5.8)

Ao contrário do PhysX (Unity), o Chaos **não expõe** "Position/Velocity Iterations"
nem "Sleep Threshold" como campos simples em Project Settings. No editor só
aparece *Velocity Iteration = 2* (foi o que o dono achou). Os demais são
constantes/CVars do engine. Valores padrão (com citação):

| Parâmetro | Default | Fonte (UE 5.8) |
|---|---|---|
| **Position Iterations** | **8** | `Chaos/PBDRigidsEvolutionGBF.h:62` (`DefaultNumPositionIterations`) |
| **Velocity Iterations** | **2** | `:63` (`DefaultNumVelocityIterations`) — bate com o que aparece na UI |
| **Projection Iterations** | **1** | `:64` (`DefaultNumProjectionIterations`) |
| **Sleep (disable) threshold** | **5 frames** | `PBDRigidsEvolutionGBF.cpp:57` (`DisableThreshold`, CVar `p.DisableThreshold2`) |

### ⭐ Shock Propagation — o diferencial arquitetural do Chaos

O `scenario-1-torre-unreal.md` (Grau A) pede documentar o Shock Propagation. Achado:
o Chaos tem shock propagation **embutido no solver de colisão e LIGADO por padrão**
— **não** é um checkbox em Project Settings, é interno (CVar). Quando ativo, ele
escala a massa inversa do corpo de baixo no contato por
`p.Chaos.PBDCollisionSolver.Position.MinInvMassScale = **0.77**`
(`Collision/PBDCollisionSolver.cpp:33`), fazendo a base "pesar mais" e distribuindo
o impulso pilha acima a cada passo.

**Isto é uma diferença cross-engine de primeira ordem:** Jolt (Godot) e PhysX-PGS
(Unity) **não têm** shock propagation e ambos **colapsaram** a torre de 100 cubos.
O Chaos é o único dos três que tem esse mecanismo por padrão → é a razão pela qual
a Torre no Unreal **pode** sustentar onde as outras duas falharam. Este é
provavelmente o achado mais importante do relatório. Não alteramos nada (default).

---

## 2. Arquitetura da implementação (tudo em código)

Módulo C++ em `Projects/physics_unreal/Source/physics_unreal/`:

| Arquivo | Papel |
|---|---|
| `BenchmarkCommon.*` | Caminhos de saída, CSV, settings globais, materiais físicos, spawn de corpos/paredes, câmera+luz |
| `BenchmarkSettings.*` | `UDeveloperSettings` — cenário e parâmetros em Project Settings > Game > Benchmark |
| `BenchmarkGameMode.*` | Dispara o cenário no BeginPlay (equivale ao autoload do Godot / RuntimeInitialize do Unity) |
| `BenchmarkActorBase.*` | Base + **medição do Physics Step Time** via tick em `TG_PrePhysics`→`TG_PostPhysics` |
| `TowerBenchmark.*` | Cenário 1 (arena + N cubos, sleep detection, debug CSV) |
| `RainBenchmark.*` | Cenário 2 (caixa + esferas, step time + FPS, reset in-place) |

Como rodar: ver `Projects/physics_unreal/BENCHMARK_SETUP.md`.

### ⚠️ Escala em centímetros

Unreal trabalha em **cm** (1 m = 100 cm). Toda a geometria é construída em cm no
código (cubo 100 cm, esfera raio 50 cm, arena 6000×6000×15000 cm, caixa
3500×8000×3500 cm, gravidade −980). **Velocidades (cm/s) e alturas (cm) são
convertidas para m/s e m ao gravar o CSV**, para os números baterem 1-pra-1 com
Godot/Unity. Sem essa conversão a comparação seria inválida.

### Fixed timestep: como garantimos 50 Hz de verdade

`MaxSubsteps=1` + `MaxSubstepDeltaTime=0.02` **sozinho não** dá 50 Hz fixo: com
FPS livre, a física andaria com o delta do frame (variável). Solução:
`bUseFixedFrameRate=true, FixedFrameRate=50` → o delta reportado é sempre 0.02,
então o Chaos avança exatamente 0.02 por frame = **50 Hz real**, equivalente ao
`FixedUpdate` do Unity e ao `physics_ticks_per_second=50` do Godot.

### Decisão de arquitetura: física ACOPLADA (sem Async Physics Tick)

O `scenario-2-chuva-unreal.md` sugere Async Physics Tick. **Optamos por NÃO usar**
e rodar a física acoplada ao frame. Razão: com async, o game thread não bloqueia
esperando a física, então **não há como medir o custo do passo no game thread** —
precisaríamos ler stats internos do Chaos (compilados fora em Shipping) ou traces
manuais do Insights (não automatizável nas 10/30 runs). Acoplado, medimos o passo
diretamente e de forma confiável (ver abaixo).

**Consequências (documentar no relatório):**
- FPS e `amostras` andam juntos (1 passo de física por frame). Em Unity/Godot
  render e física são desacoplados, então lá FPS e amostras são independentes.
- O FPS do Unreal aqui **reflete** o custo da física (ao contrário do modo async,
  onde não refletiria). Isso na verdade torna o FPS **mais comparável** com as
  outras engines, não menos.
- A métrica **primária continua sendo o Physics Step Time**, diretamente
  comparável entre as três engines.

### Métrica de step time (o risco nº 1 herdado do Unity)

O Unity perdeu uma run por `ProfilerRecorder` retornar **0.0**. Aqui o step time
vem de cercar a região de física do frame:
- o ator tica em **`TG_PrePhysics`** e carimba `StepStartCycles`;
- um `UBenchmarkPostPhysicsComponent` tica em **`TG_PostPhysics`** (após a física
  do frame ter terminado — o game thread já esperou o Chaos) e calcula
  `StepMs = (agora − StepStartCycles)`.

É o equivalente ao `Stopwatch(Physics.Simulate)` do Unity e ao
`TIME_PHYSICS_PROCESS` do Godot. **Validar na 1ª run que não vem 0** (ver
`BENCHMARK_SETUP.md` §6). Se vier 0, investigar ordem de tick / async acidental.

---

## 3. Cenário 1 — A Torre

### Setup (locks cross-engine, em cm)

| Parâmetro | Valor |
|---|---|
| Nº de cubos | 100 (config: `TowerN`) |
| Cubo | 100 cm (half-extent 50), massa 1 kg |
| Espaçamento | 100 cm exato (faces tocando) |
| Offset inicial | centro do cubo base em z=50 (base toca o chão) |
| Atrito | 0.5 (Chaos usa 1 coeficiente — ver nota abaixo) |
| Restituição | **0.0** (crítico), bounce combine Minimum |
| Arena | 6000×6000 cm, altura 15000 cm, espessura 200 cm, fechada |
| Timeout | 60 s |
| Runs | 10 (config: `TowerRunsPerN`); dataset final 30 |

**Nota atrito:** Godot/Unity usam atrito estático 0.5 + dinâmico 0.4. O
`UPhysicalMaterial` do Chaos expõe **um** `Friction`. Usamos **0.5**. Diferença
menor, documentada para o relatório.

### Métrica e CSV

- `tempo_ate_sleep_s` = `phys_frames * 0.02` (tempo de **simulação**,
  determinístico — igual a Unity/Godot). `phys_frames` também gravado.
- Sleep: `UPrimitiveComponent::RigidBodyIsAwake() == false`, checado a cada passo.
- `torre_unreal.csv`: `run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep`
- `torre_unreal_debug.csv` (por passo): `run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y`
  — `top_y` = maior Z (em m) da pilha: distingue colapso (top_y caindo) de jitter.

### Spawn: todos os N de uma vez (igual a Unity + spec)

Como no Unity (`TowerBenchmark.cs`), a coluna é spawnada **inteira em t=0**
(cubos empilhados com faces tocando, spacing 100 cm), NÃO incrementalmente — o
spec exige "perfeitamente empilhados" (pilha pré-montada). Medimos o tempo até a
pilha inteira dormir.

### Sweep de colapso ("aumentar o nº de caixas ao longo dos runs")

O que no Unity "aumenta o nº de caixas ao longo do tempo" é o **sweep** (menu
"Torre SWEEP", N = 10/15/20/25/30) — o stress test que acha **onde a pilha
colapsa**. Aqui isso é a **lista `TowerN`**. Para rodar o sweep, em
`Config/DefaultGame.ini` (ou Project Settings > Game > Benchmark):

```ini
[/Script/physics_unreal.BenchmarkSettings]
Scenario=Tower
TowerSubdir=torre-sweep
TowerRunsPerN=10
+TowerN=10
+TowerN=15
+TowerN=20
+TowerN=25
+TowerN=30
+TowerN=50
+TowerN=75
+TowerN=100
```

O harness roda N=10 (10 runs), depois N=15 (10 runs), ... cada run com mais cubos.
A coluna `num_cubos` no CSV separa cada nível. **Este sweep é o que revela se o
Shock Propagation do Chaos sustenta pilhas mais altas** que Jolt/PhysX (limiar de
colapso: Jolt ~15, PhysX-PGS ~15, PhysX-TGS ~20 — ver `results/unity/NOTES.md`).

### Reset in-place entre runs

Destrói os atores dos cubos, espera 2 passos de física, respawn (evita o crash de
reload de nível em N alto visto no Jolt).

### Resultados (sweep N=10/15/20, 10 runs — coletado)

`results/unreal/torre-sweep/` (10 runs cada):

| N | tempo até sleep (s) | σ | max_v (m/s) | topo final (m) | kept% | Veredito |
|---|---|---|---|---|---|---|
| 10 | **21.10** | ~0 | 0.56 | 9.48 / 9.5 | 100% | ✅ **ESTÁVEL** |
| 15 | **28.16** | 0 | 19.5 | 5.49 / 14.5 | 38% | ⚠️ **PARCIAL** |
| 20 | **8.26** | 0 | 19.0 | 1.50 / 19.5 | 8% | 💥 **COLAPSOU** |

**Achado 1 — determinismo do Chaos.** As 10 runs de cada N são **idênticas**
(σ ≈ 0). Chaos com timestep fixo + spawn sem aleatoriedade é 100% determinístico;
como medimos em **tempo de simulação** (não wall-clock), não há ruído. Contraste:
Godot tem σ ~0.5 (o `tempo_ate_sleep` dele é wall-clock → ruído do SO) e Unity
σ ~0.46 (não-determinismo de threading do PhysX). **Implicação p/ o método:** o
filtro ±σ é trivial no Unreal (nada a descartar); a σ das outras engines é em boa
parte artefato de medição, não física.

**Achado 2 — limiar de colapso: Chaos fica ENTRE PGS e TGS.**
- Godot/Jolt e Unity-PGS (default): colapsam já em **N=15**.
- **Chaos: sustenta N=10, PARCIAL em N=15 (assenta a ~5.5 m, não achata), colapsa em N=20.**
- Unity-TGS: sustenta N=15, colapsa em N=20.

O **N=15 PARCIAL** é a assinatura do Shock Propagation do Chaos: onde o PGS
achata reto até o chão (kept 7%), o Chaos segura metade da pilha (kept 38%) — é o
regime de "jitter no limiar" que o enunciado pede para observar. Mas **nenhum**
solver default sustenta pilhas altas: em N=20 o Chaos já colapsa como os outros.
Confirma a conclusão cross-engine: defaults não seguram torres altas rígidas.

> **N=10 leva 21 s para dormir** (max_v só 0.56 = sem colapso): a pilha é estável
> mas o Chaos converge devagar até o threshold de sleep — micro-jitter longo, não
> queda. É o outro regime do enunciado ("jitter excessivo" sem colapso).

---

## 4. Cenário 2 — A Chuva

### Setup (locks, em cm)

| Parâmetro | Valor |
|---|---|
| Esfera | raio 50 cm, massa 1 kg |
| Atrito | 0.4 | Restituição | 0.3 (bounce combine Average) |
| Variações | 1000 / 5000 / 10000 (config: `RainVariations`) |
| Caixa | 3500×8000×3500 cm, espessura 200, fechada |
| Spawn | grid 3D, spacing 115, jitter ±3 cm (seed `0xBEEF+run`) |
| Spawn Z | 2500 cm |
| Warmup | **25 passos (~0,5 s SIM)** — configurável | Janela | **500 passos (10 s SIM)** — configurável |
| Runs | 10 (config: `RainRuns`); dataset final 30 |

### ⚠️ Correção de metodologia: janela por tempo de simulação (não wall-clock)

**Bug encontrado na 1ª coleta:** com física acoplada, N alto roda abaixo do tempo
real (10k ≈ 10 FPS). A janela original de **10 s wall-clock** dava só ~2 s
*simulados* em 10k (`amostras≈104`) — as esferas ainda estavam em **queda livre**,
medindo só broadphase, não o custo real de resolver a pilha densa (o "estresse de
colisão" do enunciado). Log da 1ª coleta: 1k→10s sim, 5k→3.2s sim, 10k→2s sim.

**Correção:** warmup e janela agora contam **passos de física** (tempo simulado),
não wall-clock. Todo run/variação/engine mede o **mesmo cenário físico** (esferas
já caíram e estão empilhando). `amostras` vira fixo em 500 (= 10 s simulados); a
lentidão real fica no **FPS** (10k a ~10 FPS) e no log ("janela 10s sim = Xs reais").

**Warmup curto de propósito (corrigido):** a 1ª versão usou warmup longo (6 s)
achando que precisava pular a queda livre. Errado — quem corrige o bug de
queda-livre é a **janela sim-time** (500 passos = 10 s simulados cobrem
queda+pilha inteiras). Warmup longo só PIORA: cai na fase **assentada** (mais
barata) e subestima o custo. Então warmup volta a **~0,5 s (25 passos)**, igual a
Godot/Unity, só para descartar o transiente. Ambos configuráveis em Project
Settings > Game > Benchmark (`RainWarmupSteps`/`RainWindowSteps`) — ajuste sem
recompilar. Cálculo da queda (tudo pousa ~3,5 s) em `../../FOLLOWUP-chuva-simtime-e-30ciclos.md`.

**Alinhamento cross-engine:** Godot e Unity usam janela wall-clock e precisam da
mesma migração para sim-time + 30 ciclos → tarefa em
`FOLLOWUP-chuva-simtime-e-30ciclos.md` (código pronto para os dois).

### Métrica e CSV

- Step time: timer `TG_Pre→PostPhysics` (ver §2). FPS: `1 / delta wall-clock`
  real (não o delta fixo de 0.02).
- `chuva_unreal.csv`: `run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras`
- `amostras` = passos de física na janela de 10 s. Interpretação difere de Unity
  (ver §2, "física acoplada"): aqui amostras acompanha o FPS real.

### Resultados (10 runs, janela sim-time — coletado)

`results/unreal/chuva-default/` (média final ±σ, filtro do enunciado):

| N esferas | Step time (ms) | σ | FPS médio | amostras |
|---|---|---|---|---|
| 1.000 | **10.10** | 0.19 | 50.0 | 500 |
| 5.000 | **65.6** | 0.88 | 14.3 | 500 |
| 10.000 | **149.8** | 5.6 | 6.6 | 500 |

**A correção sim-time funcionou:** `amostras=500` em TODAS as variações (antes,
10k dava só ~104 = queda livre). O 10k pulou de ~86 ms (queda livre, medição
antiga errada) para **~150 ms** — quase o dobro — porque agora medimos a **pilha
densa** de fato (o custo real de resolver os contatos). Escalonamento Chaos:
1k→5k = 6.5×, 5k→10k = 2.3× (sublinear no topo, como o Jolt).

**vs Godot/Jolt (números antigos, wall-clock):** ~2.6 / 13.4 / 31.7 ms. O Chaos
está ~4–5× mais lento — plausível (LWC double precision + solver mais pesado +
física acoplada). **MAS:** os números do Godot/Unity são da janela wall-clock
antiga (10k deles também estava diluído por queda livre). A comparação justa só
vale depois de re-rodar Godot/Unity com a janela sim-time (já aplicada no código
deles — ver `FOLLOWUP`). Esperado: os números deles também sobem, sobretudo 10k.

> FPS: 50 (1k, mantém tempo real) → 14 (5k) → 6.6 (10k). Como a física é acoplada,
> o FPS cai junto com o custo — reflete a carga (diferente do modo async).

---

## 5. Status

| Item | Status |
|---|---|
| Projeto convertido para C++ (módulo + Target/Build.cs + .uproject) | ✅ |
| Scripts (Common, Settings, GameMode, ActorBase, Tower, Rain) | ✅ Escritos |
| Config (fixed timestep, gravity, vsync, GameMode default) | ✅ |
| Pastas de saída (`results/unreal/{torre-N100-arena,chuva-default}`) | ✅ |
| **Primeiro build** | ✅ **Compilou** (`physics_unrealEditor Win64 Development`, MSVC 14.51, 0 erros — só warnings de deprecação internos da engine) |
| Anotar solver iterations / sleep threshold / Shock Propagation (defaults) | ✅ Documentado por código-fonte (§1) |
| Validar step time ≠ 0 na 1ª run da Chuva | ✅ OK (10/65/150 ms) |
| **Dataset Torre (N=10/15/20, 10 runs)** | ✅ **Coletado** — `torre-sweep/`. Chaos: estável 10, parcial 15, colapsa 20 |
| **Dataset Chuva (1k/5k/10k, 10 runs, sim-time)** | ✅ **Coletado** — `chuva-default/`. 10.1 / 65.6 / 149.8 ms |
| **Chuva "otimizada" (CVars, exercício)** | ✅ **Coletado** — `chuva-optimized/`. 10.3 / 73.9 / **173.8 ms** → **PIOROU** (+2/+13/+16%). Cortar iterações do solver não ajuda: gargalo é colisão/contatos + 10k atores + LWC, não iterações. Ver §6 |
| Dataset final 20 runs (Unreal) | ⏳ Amanhã (dono) |
| Re-rodar Godot/Unity (Chuva sim-time + Torre 10/15/20) | ⏳ Dono roda em seguida |
| Vídeos + relatório comparativo (3 engines) | ⏳ Pendente (após Godot/Unity) |

> **Build validado:** o módulo compila limpo no VS 2026 (MSVC 14.51) contra a
> UE 5.8, 0 erros. Falta agora a validação em **runtime** (dar Play): confirmar
> que os corpos simulam, que o step time da Chuva ≠ 0, e anotar os defaults do
> solver / Shock Propagation. A metodologia e a estrutura estão prontas.

---

## 6. Chuva "otimizada" — a otimização que NÃO funcionou (e por quê)

Aplicamos, via `bRainOptimize=true` → `ApplyChaosCVars()`, um bundle de CVars do
Chaos (iterações 8→4/2→1/1→0, `Deterministic 0`, `UseCCD 0`, `DeferNarrowPhase 1`,
`IslandGroups.WorkerMultiplier 2`) em pasta separada `chuva-optimized/`. Resultado
(média final ±σ, 10 runs):

| N | default | otimizado | Δ |
|---|---|---|---|
| 1.000 | 10.10 | 10.32 | **+2,2%** |
| 5.000 | 65.59 | 73.92 | **+12,7%** |
| 10.000 | 149.83 | **173.83** | **+16,0%** |

**Piorou em tudo.** Diagnóstico (detalhado em `ANALYSIS-comparativo.md`):
- Gargalo da chuva densa = **pipeline de colisão** sobre milhares de contatos, não as
  iterações do solver. Cortar iterações raspa a parte barata.
- Menos iterações → pilha menos estável → mais corpos acordados/tremendo → **mais**
  contatos ativos por passo → solver de colisão trabalha mais.
- `WorkerMultiplier 2` numa **ilha de contato única** (pilha) só adiciona overhead de
  agendamento, sem paralelismo real.
- Custo dos ~150 ms é **estrutural** (colisão + LWC double + 10k AActors), não um
  default conservador destravável por CVar.
- Determinismo perdido de brinde (σ 10k: ≈0 → 7,5 ms com `Deterministic 0`).

**Default continua re-rodável:** `ApplyChaosCVars(false)` reseta as CVars aos padrões;
rodar default depois de otimizado na mesma sessão volta a ~10/66/150 ms.
