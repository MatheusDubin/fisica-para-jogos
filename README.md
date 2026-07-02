# Física para Jogos Digitais — Grau A & Grau B

**Benchmark comparativo de _physics engines_** — Unity (PhysX) · Godot (Jolt) · Unreal (Chaos).
Mesma cena nas três engines, medindo **estabilidade** (uma torre de cubos) e **desempenho**
(chuva de milhares de esferas), com rigor estatístico.

> **Acadêmico:** Matheus Dubin da Silveira · **Docente:** Prof.ª Rossana Baptista Queiroz
> Universidade do Vale do Rio dos Sinos (Unisinos) · Escola Politécnica · 2026

---

## ▶ Para avaliação — comece por aqui

| Entregável | Onde | Como abrir |
|---|---|---|
| **Apresentação Grau B** (principal) | [`assignment-b/slides/grau-b.html`](assignment-b/slides/grau-b.html) | abrir no **navegador** (Chrome/Edge) |
| **Apresentação Grau A** | [`assignment-b/slides/grau-a.html`](assignment-b/slides/grau-a.html) | idem |
| **Resultados & evidência** (CSVs + tabelas) | [`assignment-b/results/`](assignment-b/results/) | **comece pelo [`results/README.md`](assignment-b/results/README.md)** |
| **Guia de estudo/defesa** (conceitos + Q&A) | [`assignment-b/slides/GUIA-DE-ESTUDO-grau-b.md`](assignment-b/slides/GUIA-DE-ESTUDO-grau-b.md) | qualquer leitor de Markdown |
| **Enunciados oficiais** | [`assignments/`](assignments/) | `assignment-a.md`, `assignment-b.md` |
| **Código das 3 engines** | [`Projects/`](Projects/) | abrir cada projeto na sua engine (ver §4) |

> **Dica:** para gerar PDF dos slides, abra o `.html` no navegador com `?print-pdf` no fim da URL
> e use "Imprimir → Salvar como PDF".

---

## 1. O resultado em 3 linhas

- **Desempenho (Chuva, 10k esferas):** custo por passo de física — **Unity/PhysX 13,3 ms < Godot/Jolt
  32,8 ms < Unreal/Chaos 146,6 ms**. O PhysX é o mais barato; o Chaos custa **≈11×**.
- **Estabilidade (Torre):** **nenhuma** engine sustenta pilha rígida alta com os defaults; o **Chaos**
  (Shock Propagation) aguenta um degrau a mais (parcial em N=15). Em N=100 as três colapsam.
- **Método:** 15 execuções por configuração → M ± σ → Média Final. Dataset **canônico = n=15**.

---

## 2. Os dois trabalhos

| | **Grau A — _Physics Survey_** | **Grau B — _Benchmark Prático_** |
|---|---|---|
| Tipo | Pesquisa teórica das arquiteturas | Implementação + medição nas 3 engines |
| Entregável | Slides ([`grau-a.html`](assignment-b/slides/grau-a.html)) + pesquisa ([`assignment-a/`](assignment-a/)) | Slides ([`grau-b.html`](assignment-b/slides/grau-b.html)) + código ([`Projects/`](Projects/)) + dados ([`results/`](assignment-b/results/)) |
| Relação | Levanta o que a arquitetura **prevê** | **Mede** e confronta previsão × dado |

---

## 3. Estrutura do repositório (visão geral)

```
fisica-para-jogos/
├── README.md          ← você está aqui (porta de entrada)
├── STATUS.md          ← estado atual de cada entregável
├── FILEMAP.md         ← mapa detalhado de todos os arquivos
├── CONTEXT.md         ← contexto da disciplina e do tema
│
├── assignments/       ← enunciados oficiais + spec da cena (SCENE_SPEC.md)
├── assignment-a/      ← Grau A: pesquisa por engine + tabela comparativa + referências
├── assignment-b/      ← Grau B: slides, dados, guias e cenários
│   ├── slides/        ← grau-a.html · grau-b.html · GUIA-DE-ESTUDO-grau-b.md
│   ├── results/       ← ★ CSVs brutos + RESULTS.md + análises (comece pelo README.md)
│   ├── scenarios/     ← descrição dos 2 cenários (Torre, Chuva) por engine
│   └── howto/         ← guias de implementação por engine + método estatístico
│
└── Projects/          ← código-fonte das 3 engines (ver §4)
```

---

## 4. Código das engines (`Projects/`)

| Engine | Motor | Pasta | Versão |
|---|---|---|---|
| **Unity** | PhysX | [`Projects/physics-unity/`](Projects/physics-unity/) | 6000.5 (U6 LTS) |
| **Godot** | Jolt | [`Projects/physics/`](Projects/physics/) | 4.7 |
| **Unreal** | Chaos | **[`Projects/physics_unreal 5.8/`](Projects/physics_unreal%205.8/)** | **5.8 (canônico)** |

> **Unreal:** o projeto fica em `physics_unreal 5.8/` (`EngineAssociation: "5.8"` — a versão usada
> no benchmark, conforme os slides).

A cena de cada cenário é **100% gerada em código** (C# / GDScript / C++): posições exatas,
reprodutível, sem configuração manual de Inspector. Pontos de entrada:
`physics/scripts/` (Godot) · `physics-unity/Assets/` (Unity) · `physics_unreal 5.8/Source/` + `BENCHMARK_SETUP.md` (Unreal).

---

## 5. Documentos de navegação

- **[`STATUS.md`](STATUS.md)** — o que está pronto, o que é canônico, limitações honestas.
- **[`FILEMAP.md`](FILEMAP.md)** — mapa completo (arquivo a arquivo), marcando entregável × processo.
- **[`CONTEXT.md`](CONTEXT.md)** — disciplina, tema e como os dois trabalhos se conectam.
- **[`assignment-b/results/README.md`](assignment-b/results/README.md)** — guia de leitura dos dados
  + dicionário das colunas dos CSVs.
