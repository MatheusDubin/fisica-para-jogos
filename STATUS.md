# STATUS — Project F

> Última atualização: junho 2026
> Disciplina: Física para Jogos Digitais — Unisinos

---

## Visão Geral

| Trabalho | Fase atual | % documentação | % implementação |
|---|---|---|---|
| **Grau A** — Physics Survey | Pesquisa concluída, slides pendentes | ✅ 100% | ⏳ Slides ao final do B |
| **Grau B** — Benchmark Prático | Planejamento concluído, implementação não iniciada | ✅ 100% | ❌ 0% |

---

## Grau A — Estado Detalhado

### ✅ Concluído

- **Pesquisa por engine** (`assignment-a/research-*.md`) — 3 relatórios completos com fontes primárias:
  - Unity: PhysX 4.x como padrão, Havok descontinuado, Unity Physics DOTS como alternativa, sem destruição/fluidos nativos
  - Unreal: Chaos Physics como padrão (UE5+), Chaos Destruction production-ready (5.4+), Chaos Flesh experimental, Niagara Fluids para fluidos GPU
  - Godot: Jolt Physics como padrão (4.6+), multi-threaded nativo, sem destruição/fluidos nativos

- **Tabela comparativa** (`assignment-a/comparative-table.md`) — todos os 6 tópicos preenchidos com nomes de sistemas/componentes para as 3 engines

- **Referências por engine** (`assignment-a/references-*.md`) — fontes organizadas pelos 6 tópicos da tabela

### ⏳ Pendente

- **Slides (PDF)** — 5 seções obrigatórias (Introdução, Panorama Atual, Análise Comparativa, Considerações Finais, Referências)
  - **Decisão:** produzir apenas após o Grau B, para incorporar considerações cruzadas com os resultados

---

## Grau B — Estado Detalhado

### ✅ Concluído (documentação e planejamento)

- **Spec do assignment** (`assignments/assignment-b.md`) — enunciado + seção "O que o Grau A nos diz sobre o Grau B" com tabelas de contexto por cenário

- **Cenários genéricos** (`assignment-b/scenarios/scenario-*-generic.md`):
  - Cenário 1 (Torre): conceitos de jitter, sleep state, warm starting, shock propagation, setup, métricas, armadilhas
  - Cenário 2 (Chuva): pipeline de física (broadphase/narrowphase/solver), spiral of death, variações 1k/5k/10k, metodologia de coleta

- **Cenários por engine** (`assignment-b/scenarios/scenario-*-{unity,unreal,godot}.md`):
  - Cada arquivo tem: contexto do Grau A aplicado ao cenário, API de sleep/métricas, esboço do script de benchmark, tabela de resultados vazia

- **How-to guides** (`assignment-b/howto/`):
  - `howto-unity.md` — criação de projeto, ProfilerRecorder, IsSleeping, CSV, automação de 10 runs
  - `howto-unreal.md` — escala cm, Fixed Timestep, stat physics, Unreal Insights, FFileHelper, GameInstance
  - `howto-godot.md` — Jolt setup, Performance monitors, sleep signal, FileAccess, AutoLoad singleton
  - `howto-statistics.md` — metodologia dos 4 passos com exemplo numérico, script Python, fórmulas Excel

- **Study files** (`assignment-b/study-*.md`):
  - Análise conceitual prévia com perguntas abertas ainda não respondidas (ver seção "Pontos em Aberto" abaixo)

### ❌ Não iniciado (implementação)

- Projeto Unity (`unity/`) — cenas, scripts C#, prefabs
- Projeto Unreal (`unreal/`) — projeto UE5, classes C++, configurações
- Projeto Godot (`godot/`) — projeto 4.6, cenas .tscn, scripts GDScript
- Dados brutos (`data/raw/`) — CSVs das 10 runs por engine/variação
- Dados processados (`data/processed/`) — médias finais, desvios padrão
- `results.md` — documento consolidado de todos os resultados
- Slides/apresentação — após coleta dos dados

---

## Pontos em Aberto (do study-*.md — requerem pesquisa ou decisão)

### Unity
- [ ] `ProfilerRecorder` com marcador `"Physics.Processing"` funciona em builds não-Development? (necessário para dados limpos)
- [ ] Qual marcador exato usar: `"Physics.Processing"`, `"Physics.Simulate"` ou `"PhysicsManager.FixedUpdate"`? Confirmar no Profiler Window
- [ ] Como rodar Unity em modo headless para benchmarks sem janela gráfica?
- [ ] `Instantiate` de 10k objetos causa GC spike que distorce os dados da Chuva? Usar object pooling?

### Unreal
- [ ] `stat physics` PhysicsTime é acessível programaticamente ou apenas como texto no HUD?
- [ ] `FPhysScene_Chaos` tem API pública para ler o tempo da última step em C++?
- [ ] SpawnActor em loop para 10k objetos: causa hitch na primeira frame que invalida os dados?
- [ ] Nanite/Lumen desativados no projeto de benchmark? (adicionam custo de render que polui FPS)
- [ ] Instanced Static Mesh (ISM) com Chaos Physics funciona para física independente por instância?

### Godot
- [ ] `Performance.TIME_PHYSICS_PROCESS` é wall-clock total ou só parte do step? Verificar na documentação
- [ ] `PhysicsServer3D.body_create()` é mais rápido que `add_child` para spawnar 10k objetos?
- [ ] Flag `--headless` desativa completamente o render mantendo física ativa?
- [ ] GDScript vs C# faz diferença no Step Time (a física é C++, mas o script pode gerar overhead)?
- [ ] `FileAccess` funciona em builds exportadas? (importante para dados finais)

### Metodologia
- [ ] Definir tamanho da janela de coleta para Cenário 2 (sugestão: 10s) — padronizar nas 3 engines
- [ ] Decidir se testaremos Godot Physics (legado) como variação comparativa ao Jolt

---

## Próximas Etapas (em ordem)

1. **Resolver pontos em aberto** acima — pesquisa rápida ou teste prático
2. **Implementar Godot** — o mais automatizável (Claude Code pode gerar .tscn e project.godot)
3. **Implementar Unity** — scripts C# via Claude Code, cena mínima manual
4. **Implementar Unreal** — C++ + .ini via Claude Code, projeto criado manualmente
5. **Rodar benchmarks** — 10 runs por variação por engine
6. **Analisar dados** — script Python de estatísticas
7. **Slides A+B** — apresentação única cobrindo os dois trabalhos (ou dois PDFs separados)

---

## Decisões Tomadas

| Decisão | Motivo |
|---|---|
| Unity usa path PhysX/GameObject, não DOTS | Comparação justa com o default de cada engine |
| Godot usa Jolt Physics, não GodotPhysics | Jolt é o padrão no Godot 4.6 |
| Slides apenas após o B | Para poder responder "resultados confirmam o Grau A?" |
| Esferas para Cenário 2 (Chuva) | Shape mais simples para narrowphase; menos instabilidade que boxes |
| Fixed Timestep 50Hz (0.02s) nas 3 engines | Definido pelo assignment |
