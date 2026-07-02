# FILEMAP — Mapa de Arquivos

> Mapa completo do repositório, marcando **★ entregável** (avaliação), **⚙ código**,
> e **📄 processo** (histórico interno, não obrigatório). Porta de entrada: [`README.md`](README.md).

---

## Raiz `/`

| Arquivo | Tipo | Conteúdo |
|---|---|---|
| `README.md` | ★ | Porta de entrada — o que ler, onde, como abrir. |
| `STATUS.md` | ★ | Estado de cada entregável; canônico × exploratório × arquivado; limitações. |
| `FILEMAP.md` | — | Este arquivo. |
| `CONTEXT.md` | — | Contexto da disciplina, tema e ligação Grau A ↔ Grau B. |

---

## `/assignments/` — Enunciados oficiais

| Arquivo | Tipo | Conteúdo |
|---|---|---|
| `assignment-a.md` | ★ | Enunciado do Grau A (pesquisa teórica). |
| `assignment-b.md` | ★ | Enunciado do Grau B (benchmark) + seção "O que o Grau A diz sobre o B". |
| `SCENE_SPEC.md` | ★ | Especificação da cena de teste + seção "Desvios do spec e por quê". |

---

## `/assignment-a/` — Grau A: pesquisa e outputs

| Arquivo | Tipo | Conteúdo |
|---|---|---|
| `research-{unity,unreal,godot}.md` | ★ | Relatório de pesquisa por engine (motor default, alternativas, recursos, fontes). |
| `comparative-table.md` | ★ | Tabela comparativa consolidada das 3 engines pelos tópicos obrigatórios. |
| `references-{unity,unreal,godot}.md` | ★ | Referências por engine, organizadas pelos tópicos. |
| `references-academic.md` | ★ | Referências acadêmicas consolidadas (GDC, SIGGRAPH, etc.). |

> Os **slides** do Grau A estão em `assignment-b/slides/grau-a.html`.

---

## `/assignment-b/` — Grau B: slides, dados, guias

### Slides (entregável principal)
| Arquivo | Tipo | Conteúdo |
|---|---|---|
| `slides/grau-b.html` | ★ | **Apresentação do Grau B** (reveal.js — abrir no navegador). |
| `slides/grau-a.html` | ★ | Apresentação do Grau A. |
| `slides/GUIA-DE-ESTUDO-grau-b.md` | ★ | Glossário + conceitos + perguntas & respostas para a defesa. |

### Resultados (evidência) — `assignment-b/results/`
> **Comece pelo [`results/README.md`](assignment-b/results/README.md)** — índice + dicionário dos CSVs.

| Arquivo / pasta | Tipo | Conteúdo |
|---|---|---|
| `results/README.md` | ★ | Guia de leitura dos dados + o que cada coluna dos CSVs significa. |
| `results/RESULTS.md` | ★ | Tabelas oficiais (M, σ, faixa, descartados, **Média Final**) — n=15. |
| `results/METRICS-e-exclusoes.md` | ★ | O que cada métrica mede + o que foi excluído e por quê. |
| `results/COMPARISON-n10-vs-n15.md` | ★ | Piloto (n=10) × final (n=15) — validação de estabilidade. |
| `results/{unity,godot,unreal}/` | ★ | **CSVs brutos canônicos** (Chuva + Torre). |
| `results/_arquivo-2026-07-01-n10/` · `_arquivo-obsoleto/` | 🗄️ | Datasets arquivados (piloto n=10 / janela wall-clock antiga). |
| `results/ANALYSIS-comparativo.md` · `AUDITORIA.md` · `PLANO-CORRECOES.md` | 📄 | Análise/auditoria/plano **do piloto n=10** (histórico, com banner). |
| `results/{engine}/NOTES.md`, `godot/ANALYSIS.md` | 📄 | Notas de bastidor por engine. |

### Cenários e guias (planejamento)
| Arquivo | Tipo | Conteúdo |
|---|---|---|
| `scenarios/scenario-{1-torre,2-chuva}-generic.md` | ★ | Descrição de cada cenário (conceitos, setup, métricas). |
| `scenarios/scenario-*-{unity,unreal,godot}.md` | ★ | O cenário aplicado a cada engine (contexto do Grau A + API + script). |
| `howto/howto-{unity,unreal,godot}.md` | 📄 | Guias de implementação por engine. |
| `howto/howto-statistics.md` | ★ | Metodologia estatística (os 4 passos, com exemplo). |
| `study-{unity,unreal,godot}.md` | 📄 | Análises conceituais prévias (perguntas de estudo). |
| `HANDOFF.md` · `FOLLOWUP-*.md` · `run-checklist.html` | 📄 | Continuidade/checklist internos. |

---

## `/Projects/` — Código-fonte das 3 engines ⚙

| Pasta | Engine | Motor | Versão |
|---|---|---|---|
| `physics-unity/` | Unity | PhysX | 6000.5 |
| `physics/` | Godot | Jolt | 4.7 |
| **`physics_unreal 5.8/`** | Unreal | Chaos | **5.8** |

Pontos de entrada do código: `physics/scripts/` + `physics/scenes/` (Godot) ·
`physics-unity/Assets/` (Unity) · `physics_unreal 5.8/Source/` + `BENCHMARK_SETUP.md` (Unreal).

> As pastas de build de cada engine (`Library/`, `Binaries/`, `Intermediate/`, `Saved/`,
> `DerivedDataCache/`, `.godot/`) são geradas automaticamente — não fazem parte do código autoral.
