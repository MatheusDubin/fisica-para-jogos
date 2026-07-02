# Resultados do Benchmark — Guia de Leitura (Grau B)

> **Comece por aqui.** Este índice diz *o que ler, em que ordem, e o que cada arquivo/CSV
> significa.* Dataset **canônico = n=15** (15 execuções por configuração). Método do enunciado:
> 15 runs → M e σ (populacional, ÷n) → descartar fora de `[M−σ, M+σ]` → **Média Final**.

---

## 1. TL;DR — o resultado em 5 linhas

- **Chuva (desempenho):** custo por passo de física, em 10k esferas — **Unity/PhysX 13,3 ms
  < Godot/Jolt 32,8 ms < Unreal/Chaos 146,6 ms**. O PhysX é o mais barato; o Chaos custa **≈11×**.
- **Torre (estabilidade):** **nenhuma** engine sustenta pilha rígida alta com os defaults. O
  **Chaos** (Shock Propagation) aguenta **1 degrau a mais** (parcial em N=15); em N=100 as três colapsam.
- **Métrica que compara:** Chuva → **Physics Step Time**; Torre → **colapso (`kept%`)**. FPS e
  tempo-até-sleep **não** comparam 1-a-1 (explicado abaixo).
- **Reprodutibilidade:** um piloto n=10 e o final n=15 dão a mesma classificação (Chuva se moveu < 7%).

---

## 2. Ordem de leitura sugerida

| Ordem | Arquivo | O que é |
|---|---|---|
| 1 | **`../slides/grau-b.html`** | A apresentação (abrir no navegador). O trabalho contado do início ao fim. |
| 2 | **`RESULTS.md`** | Tabelas oficiais: valores brutos + M, σ, faixa [M±σ], descartados e **Média Final** de cada config (n=15). Gerado por `aggregate.js`. |
| 3 | **`METRICS-e-exclusoes.md`** | O que cada métrica mede, como se relacionam, e exatamente o que foi excluído (e por quê). |
| 4 | **`COMPARISON-n10-vs-n15.md`** | Piloto (n=10) × final (n=15) lado a lado — prova de que o número é estável. |
| 5 | *(opcional)* **`../slides/GUIA-DE-ESTUDO-grau-b.md`** | Glossário + conceitos + perguntas & respostas, para estudo. |

Os **CSVs brutos** (pastas `unity/`, `godot/`, `unreal/`) são a evidência por trás das tabelas —
qualquer número do `RESULTS.md` pode ser reconferido neles. Dicionário de colunas na §4.

---

## 3. Mapa das pastas de dados

### ✅ Canônico (n=15, entra na comparação principal)
```
unity/   godot/   unreal/
 ├── chuva-default/       → Cenário 2 (Chuva): 1k / 5k / 10k esferas
 ├── torre-N100-arena/    → Cenário 1 (Torre): pilha de 100 cubos
 └── torre-sweep/         → Cenário 1 (Torre): varredura N = 10 / 15 / 20
```

### 🔬 Exploratório (rotulado à parte no `RESULTS.md` — NÃO é a coluna principal)
Rodadas deliberadas de "e se…", mantidas para honestidade, fora da comparação default:
- `unreal/chuva-optimized/` — bundle de CVars do Chaos (menos iterações, sem CCD…). **Piorou** o
  10k (146,6 → 173,8 ms): mostra que o gargalo é a colisão, não as iterações.
- `unity/torre-*-tgs/` — solver **TGS** (opt-in) em vez do PGS default: sobe 1 degrau (segura N=15).
- `unity/torre-*-tuned/` — `solverIterations` 6→20: **não** move o limiar de colapso.
- `unity/torre-sweep-default/` — sweep exploratório n=3 (só sanity-check).

### 🗄️ Arquivado (fora do agregador — guardado só como histórico)
- `_arquivo-2026-07-01-n10/` — o **piloto n=10** (usado apenas pelo `COMPARISON-n10-vs-n15.md`).
- `_arquivo-obsoleto/` — Chuva com janela **wall-clock** antiga (bug: 10 s reais ≈ 2 s simulados
  em 10k). Substituída pela janela por **tempo de simulação**. **Não usar.**

---

## 4. Dicionário dos CSVs (o que cada coluna significa)

### `{engine}/chuva-default/chuva_{engine}.csv` — Cenário 2 (Chuva)
`run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras`

| Coluna | Significado |
|---|---|
| `run` | Índice da execução (1…15). |
| `variacao` | Nº de esferas: 1000 / 5000 / 10000. |
| **`physics_step_ms_medio`** | **Tempo de CPU (ms) para avançar 1 passo de física (0,02 s) com N corpos. → MÉTRICA PRIMÁRIA (compara entre engines).** |
| `physics_step_ms_max` | Pior passo isolado da janela (pico quando a pilha se forma). Diagnóstico. |
| `fps_medio` | FPS do laço de render. ⚠️ **Não** compara entre engines (ver §5). |
| `amostras` | Nº de passos de física na janela (**alvo 500** = 10 s simulados). Confirma que as 3 mediram o mesmo trecho. |

### `{engine}/torre-*/torre_{engine}.csv` — Cenário 1 (Torre), resumo por run
`run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep`

| Coluna | Significado |
|---|---|
| `run` | Índice da execução. |
| `num_cubos` | N da pilha (10 / 15 / 20 no sweep; 100 no N100). |
| `tempo_ate_sleep_s` | Tempo até **todos** os corpos dormirem. ⚠️ **Não** compara 1-a-1 (limiar de sleep difere por engine — ver §5). |
| `timeout` | `1` = bateu 60 s sem dormir. **Resultado válido**, não outlier. |
| `phys_frames` | Nº de passos de 0,02 s executados (auditoria de duração). |
| `max_v` | Pico de velocidade (m/s). Distingue *pancaking* (~40, queda) de explosão (altíssimo). |
| `t_primeiro_sleep` / `t_metade_sleep` | Progressão do sleep (1º corpo / metade da pilha). |

### `{engine}/torre-*/torre_{engine}_debug.csv` — Cenário 1 (Torre), trace por frame
`run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y`
Um registro **por passo de física** (arquivo grande). Colunas-chave: `top_y` = altura do topo da
pilha naquele instante; `asleep`/`active_objs` = quantos corpos dormiram/seguem ativos.

> **De onde vem o `kept%` (métrica primária da Torre)?** Não é uma coluna — é **derivado** do
> `top_y` final deste trace: `kept% = top_y_final ÷ (num_cubos − 0,5)`. É **geométrico**, então
> **independe do limiar de sleep** → é o único critério de estabilidade comparável entre engines.
> Veredito: **ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%**. As tabelas de `kept%` já prontas
> estão no `RESULTS.md` (seção "Estabilidade da Torre").

---

## 5. Duas ressalvas que você vai querer entender antes de comparar

1. **FPS não compara entre engines.** Na Unreal a física é **acoplada** ao frame (1 passo/frame,
   teto 50 Hz) → o FPS reflete o custo da física. Em Unity/Godot o render é **desacoplado e livre**
   → ali "FPS" é velocidade de GPU (ex.: Godot faz 1695 FPS rodando a física a 50 Hz). Por isso o
   **Physics Step Time** é a métrica de comparação, e o FPS é lido só como tendência por engine.

2. **Tempo-até-sleep não compara entre engines.** Cada engine declara "dormiu" a um limiar
   diferente (Unity 0,005 m/s · Jolt 0,03 m/s · Chaos por 5 frames). Comparar esse tempo mede
   *definição de sleep*, não física. Por isso a Torre é lida por **colapso (`kept%`)**.

Detalhamento completo (incluindo os casos de **distribuição bimodal** e **timeout**, onde a Média
Final perde sentido e usamos mediana + grupos) está no `METRICS-e-exclusoes.md`.

---

## 6. Documentos de processo (contexto interno — NÃO são o número canônico)

Estes registram o *caminho* até o resultado (rigor, auto-auditoria, correções). São úteis como
histórico, mas **os números finais estão no `RESULTS.md` (n=15)**, não neles:

| Arquivo | O que é | Status |
|---|---|---|
| `ANALYSIS-comparativo.md` | Análise crítica escrita sobre o **piloto n=10** | histórico — números superados pelo n=15 |
| `AUDITORIA.md` | Auto-auditoria cética feita **antes** da coleta n=15 | histórico — a maioria dos achados **já foi corrigida** no n=15 |
| `PLANO-CORRECOES.md` | Tracker interno das correções derivadas da auditoria | histórico |
| `{engine}/NOTES.md`, `godot/ANALYSIS.md` | Notas de bastidor por engine | histórico |

> A `AUDITORIA.md` é, na prática, uma prova de rigor: o próprio autor listou as fraquezas do piloto
> (Unreal N=100 ausente, PGS≠TGS, datasets obsoletos misturados) e o dataset n=15 as **resolveu**.
