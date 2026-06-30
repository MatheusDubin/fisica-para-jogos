# FILEMAP — Project F

> Mapa completo de todos os arquivos do projeto com descrição do conteúdo.
> Usar como referência ao instruir agentes sobre onde ler/escrever.

---

## Raiz `/`

```
Project-F/
├── CONTEXT.md          ← visão geral do projeto, estrutura, abordagem multi-agente
├── STATUS.md           ← estado atual: o que está feito, o que falta, pontos em aberto
├── FILEMAP.md          ← este arquivo
└── assignments/        ← specs oficiais dos dois trabalhos
└── assignment-a/       ← outputs da pesquisa do Grau A
└── assignment-b/       ← planejamento e (futuramente) implementação do Grau B
```

---

## `/assignments/` — Specs dos Trabalhos

| Arquivo | Conteúdo |
|---|---|
| `assignment-a.md` | Enunciado completo do Grau A + plano de execução em 3 fases (pesquisa, artigos, slides) + checklist de status |
| `assignment-b.md` | Enunciado completo do Grau B + **seção "O que o Grau A nos diz sobre o Grau B"** com tabelas de contexto por cenário + plano de execução em 5 fases + checklist de status |

> `assignment-b.md` é o ponto de entrada principal para qualquer agente que for trabalhar no Grau B. Leia-o primeiro.

---

## `/assignment-a/` — Grau A: Pesquisa e Outputs

| Arquivo | Conteúdo |
|---|---|
| `research-unity.md` | Relatório completo de pesquisa sobre Unity: motor padrão (PhysX 4.x), alternativas (Unity Physics DOTS, Havok descontinuado), tabela de recursos com nomes de sistemas, pontos fortes/fracos, referências |
| `research-unreal.md` | Relatório completo sobre Unreal Engine: Chaos Physics (XPBD), histórico PhysX→Chaos, tabela de recursos (Chaos Destruction, Chaos Cloth, Chaos Flesh, Niagara Fluids), pontos fortes/fracos, referências |
| `research-godot.md` | Relatório completo sobre Godot: Jolt Physics (padrão 4.6+) vs Godot Physics (legado), tabela de recursos, SoftBody3D, sem destruição/fluidos nativos, pontos fortes/fracos, referências |
| `comparative-table.md` | **Tabela comparativa consolidada** das 3 engines pelos 6 tópicos obrigatórios: corpos rígidos, juntas, soft bodies, destruição, fluidos, arquitetura. Inclui tabela de pontos fortes/fracos e notas para slides |
| `references-unity.md` | Referências do Unity organizadas pelos 6 tópicos (docs oficiais + fontes acadêmicas) |
| `references-unreal.md` | Referências do Unreal organizadas pelos 6 tópicos (docs oficiais + fontes acadêmicas) |
| `references-godot.md` | Referências do Godot organizadas pelos 6 tópicos (docs oficiais + fontes acadêmicas) |
| `references-academic.md` | **Referências acadêmicas consolidadas** (GDC, SIGGRAPH, SBGames, arXiv, IEEE) pelos 6 tópicos — com resumos. Fase 2 do Assignment A. |

> Para produzir os slides do Grau A, ler: `comparative-table.md` (conteúdo), `research-*.md` (detalhes), `references-*.md` (citações).

---

## `/assignment-b/scenarios/` — Cenários de Benchmark

### Arquivos Genéricos (independentes de engine)

| Arquivo | Conteúdo |
|---|---|
| `scenario-1-torre-generic.md` | O que o cenário testa (jitter, sleep state, solver convergência), conceitos de warm starting / shock propagation / Baumgarte stabilization, setup detalhado (parâmetros, geometria, pseudocódigo), metodologia de coleta, tabela de resultados template, armadilhas comuns |
| `scenario-2-chuva-generic.md` | O que o cenário testa (broadphase, narrowphase, solver throughput), conceito de spiral of death, Physics Step Time vs FPS, variações 1k/5k/10k, metodologia de coleta automatizada, armadilhas comuns, como interpretar escalabilidade |

### Arquivos por Engine — Cenário 1 (Torre)

| Arquivo | Conteúdo |
|---|---|
| `scenario-1-torre-unity.md` | **Contexto do Grau A** (PhysX Gauss-Seidel, warm starting, sem Shock Propagation, dois paths de física) + configuração Unity + API `IsSleeping()` + esboço do script `TowerBenchmark.cs` + tabela de resultados vazia |
| `scenario-1-torre-unreal.md` | **Contexto do Grau A** (Chaos XPBD, Shock Propagation — verificar se ativo, escala cm) + configuração UE + API `OnComponentSleep` + esboço do `TowerBenchmarkActor` + tabela de resultados vazia |
| `scenario-1-torre-godot.md` | **Contexto do Grau A** (Jolt impulse-based, multi-threading limitado em pilhas estáticas, comparação com GodotPhysics legado) + configuração Godot + API `rb.sleeping` + signal `sleeping_state_changed` + esboço do `tower_benchmark.gd` + tabela de resultados vazia |

### Arquivos por Engine — Cenário 2 (Chuva)

| Arquivo | Conteúdo |
|---|---|
| `scenario-2-chuva-unity.md` | **Contexto do Grau A** (PhysX SAP/MBP broadphase, threading parcial, FPS acoplado, por que não DOTS) + configuração Unity + API `ProfilerRecorder` para Physics Step Time + esboço do `RainBenchmark.cs` + tabelas de resultados vazias por variação |
| `scenario-2-chuva-unreal.md` | **Contexto do Grau A** (Chaos islands paralelas, Async Physics Tick desacopla FPS, LWC double precision eleva custo base) + configuração UE + `stat physics` + Unreal Insights + esboço do `RainBenchmarkActor` + tabelas de resultados vazias |
| `scenario-2-chuva-godot.md` | **Contexto do Grau A** (Jolt job system nativo, FPS acoplado ao game loop, TIME_PHYSICS_PROCESS inclui job scheduling) + configuração Godot + `Performance.TIME_PHYSICS_PROCESS` + esboço do `rain_benchmark.gd` + AutoLoad RunManager + tabelas de resultados vazias |

---

## `/assignment-b/howto/` — Guias Práticos por Engine

> Use estes arquivos quando for implementar. São guias passo a passo para quem está enferrujado na engine.

| Arquivo | Conteúdo |
|---|---|
| `howto-unity.md` | Criar projeto, Fixed Timestep, anotar padrões de Physics, Profiler Window, `ProfilerRecorder` com código completo, `IsSleeping()`, `DontDestroyOnLoad` singleton para 10 runs, `StreamWriter` para CSV, criação de prefabs físicos via Inspector, checklist, referência rápida de APIs |
| `howto-unreal.md` | Escala cm (tabela de conversão), criar projeto, Fixed Timestep, anotar padrões, `stat physics` no console, Unreal Insights passo a passo, `OnComponentSleep` em C++ e Blueprint, `FFileHelper` para CSV, `UGameInstance` para persistir runs, criação de objetos físicos, verificar Shock Propagation, checklist, referência rápida |
| `howto-godot.md` | Criar projeto, confirmar Jolt, Fixed Timestep, V-Sync, `Performance.get_monitor()` com código, `Engine.get_frames_per_second()`, `rb.sleeping` e signal, `FileAccess` para CSV em `user://`, AutoLoad `RunManager` com código completo, criação de objetos via código, Profiler do editor vs builds, exportar build, checklist, referência rápida |
| `howto-statistics.md` | Os 4 passos da metodologia explicados, exemplo numérico completo (média → variância → σ → filtro → média final), template de tabela para preencher, script Python completo (`calcular_stats.py`), formato CSV esperado, fórmulas Excel/Sheets, casos especiais (muitas runs descartadas, torre que não dorme) |

---

## `/assignment-b/study-*.md` — Análises Conceituais Prévias

> Arquivos de estudo gerados antes dos howto guides. Contêm análise conceitual e **perguntas abertas** ainda não respondidas. Consultar ao iniciar a implementação.

| Arquivo | Conteúdo relevante |
|---|---|
| `study-unity.md` | Análise do Fixed Timestep, discussão PhysX vs DOTS, armadilhas (GC, object pooling), **perguntas abertas** sobre ProfilerRecorder em builds, modo headless |
| `study-unreal.md` | Análise do Fixed Timestep vs Async Physics, ISM com Chaos, armadilhas (SpawnActor overhead, LWC, Nanite/Lumen), **perguntas abertas** sobre acesso programático ao PhysicsTime |
| `study-godot.md` | Análise Jolt vs GodotPhysics, `PhysicsServer3D.body_create()` como alternativa mais rápida ao `add_child`, `--headless` flag, **perguntas abertas** sobre semântica de TIME_PHYSICS_PROCESS |

---

## Arquivos Futuros (ainda não criados)

```
Project-F/
├── unity/                          ← projeto Unity (a criar)
│   ├── Assets/Scripts/
│   │   ├── TowerBenchmark.cs
│   │   ├── RainBenchmark.cs
│   │   └── RunManager.cs
│   └── Assets/Scenes/
├── unreal/                         ← projeto UE5 (a criar)
│   ├── Source/ProjectFUnreal/
│   │   ├── TowerBenchmarkActor.h/.cpp
│   │   └── RainBenchmarkActor.h/.cpp
│   └── Config/DefaultEngine.ini
├── godot/                          ← projeto Godot 4.6 (a criar — pode ser gerado por Claude)
│   ├── project.godot
│   ├── scenes/
│   │   ├── tower_benchmark.tscn
│   │   └── rain_benchmark.tscn
│   └── scripts/
│       ├── tower_benchmark.gd
│       ├── rain_benchmark.gd
│       └── run_manager.gd
├── data/
│   ├── raw/                        ← CSVs brutos das 10 runs
│   │   ├── torre_unity.csv
│   │   ├── torre_unreal.csv
│   │   ├── torre_godot.csv
│   │   ├── chuva_unity.csv
│   │   ├── chuva_unreal.csv
│   │   └── chuva_godot.csv
│   └── processed/                  ← saída do calcular_stats.py
│       └── results_summary.csv
├── results.md                      ← documento consolidado com todas as tabelas
└── calcular_stats.py               ← script Python de análise estatística
```

---

## Resumo para Agentes

**Para trabalhar no Grau A:**
- Ler: `assignments/assignment-a.md`, `assignment-a/comparative-table.md`, `assignment-a/research-*.md`
- Escrever: slides (novo arquivo a criar em `assignment-a/`)

**Para trabalhar no Grau B — Cenário 1 (Torre):**
- Ler: `assignments/assignment-b.md` (seção Grau A), `assignment-b/scenarios/scenario-1-torre-generic.md`, `assignment-b/scenarios/scenario-1-torre-{engine}.md`, `assignment-b/howto/howto-{engine}.md`
- Escrever: scripts em `{engine}/`

**Para trabalhar no Grau B — Cenário 2 (Chuva):**
- Ler: `assignments/assignment-b.md` (seção Grau A), `assignment-b/scenarios/scenario-2-chuva-generic.md`, `assignment-b/scenarios/scenario-2-chuva-{engine}.md`, `assignment-b/howto/howto-{engine}.md`
- Escrever: scripts em `{engine}/`

**Para análise de dados:**
- Ler: `assignment-b/howto/howto-statistics.md`, `data/raw/*.csv`
- Escrever: `data/processed/`, `results.md`

**Para os slides finais:**
- Ler: `assignment-a/comparative-table.md`, `assignment-a/research-*.md`, `results.md`, `assignments/assignment-a.md` (estrutura), `assignments/assignment-b.md` (considerações críticas)
