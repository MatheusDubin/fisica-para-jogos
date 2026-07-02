# STATUS — Estado do Projeto

> Última atualização: **2026-07-01** · Disciplina: Física para Jogos Digitais — Unisinos
> Índice de navegação em [`README.md`](README.md).

## Visão geral

| Trabalho | Estado | Entregáveis |
|---|---|---|
| **Grau A** — Physics Survey | ✅ **Concluído** | Pesquisa ([`assignment-a/`](assignment-a/)) + slides ([`slides/grau-a.html`](assignment-b/slides/grau-a.html)) |
| **Grau B** — Benchmark Prático | ✅ **Concluído** | Código (3 engines) + dados (n=15) + análise + slides ([`slides/grau-b.html`](assignment-b/slides/grau-b.html)) |

---

## Grau B — detalhamento

| Item | Estado |
|---|---|
| Implementação nas 3 engines (Unity/PhysX, Godot/Jolt, Unreal/Chaos) | ✅ feito — [`Projects/`](Projects/) |
| Cenário 1 — Torre (N=100 + varredura N=10/15/20), **nas 3 engines** | ✅ coletado |
| Cenário 2 — Chuva (1k / 5k / 10k esferas), **nas 3 engines** | ✅ coletado |
| Coleta estatística: **15 execuções por config** → M ± σ → Média Final | ✅ feito |
| Resultados consolidados | ✅ [`results/RESULTS.md`](assignment-b/results/RESULTS.md) |
| Slides (7 seções) + guia de estudo | ✅ feito |

**Dataset canônico = n=15.** Um piloto anterior de **n=10** foi mantido arquivado
(`results/_arquivo-2026-07-01-n10/`) e serve apenas à comparação de reprodutibilidade
([`COMPARISON-n10-vs-n15.md`](assignment-b/results/COMPARISON-n10-vs-n15.md)) — subir de 10→15
execuções moveu a Chuva em < 7% e **não mudou nenhuma conclusão**.

---

## O que é canônico × exploratório × arquivado (para não confundir)

- ✅ **Canônico (n=15):** `results/{unity,godot,unreal}/{chuva-default,torre-N100-arena,torre-sweep}/`
  + `RESULTS.md`, `COMPARISON-n10-vs-n15.md`, `METRICS-e-exclusoes.md`, os slides e o guia.
- 🔬 **Exploratório (fora da comparação principal, rotulado):** `unreal/chuva-optimized/` (otimização
  que piorou), `unity/torre-*-tgs/` e `unity/torre-*-tuned/` (solver/iterações alternativos).
- 🗄️ **Arquivado (histórico, não usar):** `_arquivo-2026-07-01-n10/` (piloto) e `_arquivo-obsoleto/`
  (janela wall-clock antiga, substituída pela janela por tempo de simulação).
- 📄 **Docs de processo (histórico, não são o número final):** `results/ANALYSIS-comparativo.md`,
  `AUDITORIA.md`, `PLANO-CORRECOES.md`, `{engine}/NOTES.md` — todos marcados com banner. Ver
  [`results/README.md`](assignment-b/results/README.md) §6.

---

## Limitações honestas (declaradas no trabalho)

- A "diferença de ~11×" do Chaos é **custo bruto da configuração** (editor + física acoplada +
  sincronização de 10k atores + LWC), **não** uma medição isolada do solver.
- **FPS não é comparável 1-a-1** entre engines (render acoplado na Unreal vs livre em Unity/Godot).
- **Tempo-até-sleep não compara** entre engines (limiar de sleep difere: 0,005 · 0,03 · 5 frames)
  → a Torre é lida por **colapso (`kept%`)**.
- O **FPS 10k do Godot é bimodal** → reportado por mediana + grupos, não pela Média Final.

---

## Correções aplicadas ao longo do processo (prova de rigor)

- Janela **wall-clock → tempo de simulação** (500 passos): o Chaos 10k saltou de ~86 para ~147 ms.
- Unity: `ProfilerRecorder` retornava 0 → medição via `Stopwatch(Physics.Simulate)`.
- Correção teórica: o solver default do Unity é **PGS**, não TGS.
- Jolt: buffers ajustados + reset in-place → 10k roda sem travar.
- Unreal Torre **N=100 coletado** (era a lacuna apontada na auto-auditoria).

---

## Possíveis próximos passos (não bloqueiam a entrega)

- Publicar o repositório no GitHub (inserir o link nos slides) — atenção ao `.gitignore` das pastas
  de build das engines (`Library/`, `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`).
- Gravar os clipes de vídeo (Torre colapsando · Chuva 10k) referenciados nos slides.
- Chaos em build **Shipping + Async Physics Tick** para isolar o custo puro do solver.
