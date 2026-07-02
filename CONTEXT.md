# Contexto do Projeto

## Disciplina
**Física para Jogos Digitais** — Universidade do Vale do Rio dos Sinos (Unisinos), Escola Politécnica.
Acadêmico: **Matheus Dubin da Silveira** · Docente: **Prof.ª Rossana Baptista Queiroz** · 2026.

## Tema central
Comparar os **_physics engines_** embutidos nas três principais game engines, colocando a
arquitetura interna de cada uma à prova em cenas idênticas:

- **Unity** — PhysX (solver PGS)
- **Godot** — Jolt (impulse-based / Gauss-Seidel)
- **Unreal** — Chaos (XPBD + Shock Propagation)

## Os dois trabalhos e como se conectam

| | Grau A — Physics Survey | Grau B — Benchmark Prático |
|---|---|---|
| **Pergunta** | O que a arquitetura de cada motor **prevê**? | O que os testes **medem** na prática? |
| **Como** | Pesquisa das arquiteturas (solvers, threading, CCD, destruição, fluidos) | Implementação das mesmas cenas nas 3 engines + coleta estatística |
| **Saída** | `assignment-a/` + `slides/grau-a.html` | `Projects/` + `assignment-b/results/` + `slides/grau-b.html` |

O Grau B fecha o ciclo: cada previsão do Grau A é **confrontada com o dado medido** (ex.: o Chaos é
mesmo o mais caro? O Shock Propagation ajuda a torre? O Jolt escala melhor?). A reconciliação
previsão × realidade está na seção "Análise Crítica" dos slides do Grau B.

## Os dois cenários de teste

- **Cenário 1 — A Torre:** uma pilha alta de cubos rígidos testa a **estabilidade** do solver
  (converge? colapsa? treme?). Métrica primária: **colapso (`kept%`)**.
- **Cenário 2 — A Chuva:** 1k / 5k / 10k esferas caindo numa caixa fechada testam o **desempenho**
  sob estresse de colisão. Métrica primária: **Physics Step Time** (ms por passo).

Regra de comparação justa: só o **motor de física** muda entre as engines — timestep (0,02 s / 50 Hz),
gravidade, massa, atrito, formas e a cena (gerada em código) são **idênticos** nos três.

## Método
15 execuções por configuração → média (M) e desvio padrão populacional (σ) → descartar fora de
`[M − σ, M + σ]` → **Média Final**. Detalhes e casos especiais (bimodal, timeout) em
[`assignment-b/results/METRICS-e-exclusoes.md`](assignment-b/results/METRICS-e-exclusoes.md).

> Para navegar o repositório, veja [`README.md`](README.md) (porta de entrada),
> [`STATUS.md`](STATUS.md) (estado) e [`FILEMAP.md`](FILEMAP.md) (mapa de arquivos).
