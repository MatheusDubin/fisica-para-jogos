# Métricas & Exclusões — Benchmark Grau B (n=15)

Guia de referência: **o que cada métrica mede, como elas se relacionam, e exatamente
quais dados são excluídos** (pelo método estatístico do enunciado e pelas decisões de
escopo). Números citados são do dataset canônico n=15 (`RESULTS.md`).

---

## 1. As métricas — o que cada uma mede

### Cenário 1 — A Torre (estabilidade do solver)

| Métrica (coluna CSV) | Unidade | O que mede | Comparável entre engines? |
|---|---|---|---|
| `tempo_ate_sleep_s` | s (tempo de **simulação**) | Tempo até **todos** os N corpos entrarem em *sleep*. Reflete convergência do solver. | ❌ **Não** — cada engine dorme a um limiar diferente (ver §3.6). |
| **`kept%` (colapso)** | % | Altura final do topo ÷ altura se intacta (`top_y_final / (N−0.5)`). Geométrica. | ✅ **Sim** — independe do limiar de sleep. **Métrica primária da Torre.** |
| `max_v` | m/s | Pico de velocidade de qualquer corpo na run. Distingue *pancaking* de explosão. | ✅ Sim (diagnóstico). |
| `phys_frames` | passos | Nº de passos de 0,02 s executados (auditoria de duração). | — |
| `t_primeiro_sleep` / `t_metade_sleep` | s | Progressão do sleep (1º corpo / metade da pilha). | ❌ (mesma razão do tempo total). |
| `timeout` | 0/1 | 1 = atingiu 60 s sem todos dormirem. **Resultado válido** (ver §3.2). | — |

> **Veredito de colapso:** ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%.

### Cenário 2 — A Chuva (performance sob estresse)

| Métrica (coluna CSV) | Unidade | O que mede | Comparável entre engines? |
|---|---|---|---|
| **`physics_step_ms_medio`** | ms | Tempo de CPU para avançar **um** passo de física (0,02 s) com N corpos. | ✅ **Sim** — mesma grandeza física nas 3. **Métrica primária da Chuva.** |
| `physics_step_ms_max` | ms | Pior passo isolado da janela (o pico quando a pilha se forma). | ✅ Sim (diagnóstico de spike). |
| `fps_medio` | fps | Ritmo do laço de **render**. | ❌ **Não** — depende do acoplamento render↔física (ver §2). |
| `amostras` | contagem | Nº de passos de física na janela de coleta (alvo **500** = 10 s simulados). | Auditoria (ver §2). |

---

## 2. Como as métricas se relacionam

- **`physics_step_ms_medio` ↔ `fps_medio` (por que uma compara e a outra não):**
  na Unreal a física é **acoplada** ao frame (1 passo/frame) → `fps ≈ 1000/step_ms`,
  com teto de 50 (Fixed Frame Rate). Em Unity/Godot render e física são **desacoplados**
  → o "FPS" é velocidade de render/GPU, **não** custo de física (ex.: Godot 1k = 1695 fps
  com física a 50 Hz). Por isso o **Step Time** compara e o **FPS** não.

- **`physics_step_ms_medio` ↔ `amostras`:** a janela é por **tempo de simulação**
  (500 passos), não wall-clock. `amostras=500` em todas confirma que as 3 mediram o
  **mesmo trecho físico** (queda + pilha densa). Se a física roda abaixo do tempo real
  (Unreal 10k ≈ 6,5 fps), a janela ainda captura 500 passos — só leva mais segundos reais.

- **`kept%` ↔ `tempo_ate_sleep_s`:** ambos medem estabilidade, mas `kept%` é
  **independente do limiar** (comparável) e o tempo-até-sleep **não** é. Exemplo gritante:
  Godot sweep N=10 tem `kept=100%` (estável) porém tempos de 0,5 s a 11 s na mesma config
  — o tempo é ruidoso, o colapso é limpo.

- **`kept%` ↔ `max_v`:** colapso (`kept` baixo) com `max_v` moderado (~40 m/s, compatível
  com queda sob gravidade) = **pancaking** (achatamento reto). `max_v` altíssimo com `kept`
  baixo seria **explosão**. Os nossos são pancaking.

- **Pipeline estatístico `M → σ → Média Final`:** ver §3.1.

---

## 3. Dados EXCLUÍDOS — o quê, por quê, onde

### 3.1 Filtro ±σ (exigência do enunciado)

Por configuração: `M` = média, `σ` = desvio populacional (÷n). **Descarta-se toda run
fora de `[M−σ, M+σ]`**; a **Média Final** é a média só das que sobraram
(`aggregate.js` / `howto-statistics.md`).

> ⚠️ **Descartar ~1/3 das runs é ESPERADO, não um sinal de dado ruim.** A faixa ±1σ, numa
> distribuição normal, contém ~68% dos valores — logo ~32% caem fora **por definição**.
> Descartes n=15 (Step Time): Godot 5/5/5 · Unity 5/1/1 · Unreal 3/7/4.

> ⚠️ **σ minúsculo → muitos descartes, porém inofensivos.** Quando a engine é ultra-consistente
> (ex.: Unity Chuva 1k FPS: 230–235, σ=1,4), a faixa fica estreitíssima e o filtro descarta
> 7/15 runs — mas como **todos os valores são quase idênticos**, a Média Final (232,7) é
> praticamente igual à média bruta. Contagem de descarte alta ≠ instabilidade.

### 3.2 Timeouts — resultado VÁLIDO, não outlier

Runs que batem os 60 s sem dormir (ex.: Godot sweep N=20 run 10 = 60,018 s) são um
**resultado físico legítimo** (a pilha nunca assenta), não um outlier a remover. Entram no
cálculo. Como a Torre é lida por **colapso**, não por tempo, não distorcem a conclusão.

### 3.3 Distribuições bimodais → Média Final SEM significado → mediana + clusters

Quando os valores formam **dois grupos**, a Média Final não representa nenhum deles.
Caso real: **Godot Chuva 10k FPS** = `98, 26, 14, 10, 9, 10, 17, 12, 53, 10, 42, 17, 11, 10, 11`
(um cluster ~10, alguns 40–98). A faixa ±σ chega a incluir **limite negativo** (`[−0,2 ; 46,8]`).
→ Nesses casos reportamos **mediana + contagem de clusters**, não a Média Final.

### 3.4 Colunas EXPLORATÓRIAS — fora da comparação principal (mas guardadas)

Rodadas deliberadas de "e se…", mantidas em subpastas próprias e **rotuladas à parte** no
`RESULTS.md`; **não** entram na comparação default das 3 engines:

| Config | O que é | Achado |
|---|---|---|
| `unreal/chuva-optimized` | Bundle de CVars do Chaos (menos iter, sem CCD…) | **Piorou**: 10k 146,6 → 173,8 ms (+18,7%). O gargalo é a colisão, não as iterações. |
| `unity/torre-N100-tuned`, `torre-sweep-tuned` | `solverIterations=20` (vs default 6) | Não move o limiar de colapso (N=20 ainda colapsa). |
| `unity/torre-sweep-tgs`, `torre-N100-tgs` | Solver **TGS** (opt-in) em vez de PGS | Sobe **1 degrau**: segura N=15 (100%), colapsa em N=20. |
| `unity/torre-sweep-default` | Sweep exploratório n=3 | Amostra pequena; só sanity-check. |

### 3.5 Dados OBSOLETOS — arquivados, fora do agregador

O `aggregate.js` só varre `results/{godot,unity,unreal}/**`. Ficam **fora do glob** (não
poluem os números):

| Pasta | Conteúdo | Por que saiu |
|---|---|---|
| `_arquivo-obsoleto/` | Chuva com janela **wall-clock** antiga | Bug: 10 s reais = ~2 s simulados em 10k (media só queda livre). Substituída por janela sim-time. |
| `_arquivo-2026-07-01-n10/` | Batch **n=10** anterior | Superado pelo n=15. Guardado só para o `COMPARISON-n10-vs-n15.md`. |

### 3.6 Exclusões por SPEC / método (os "Desvios do spec")

- **Sleep threshold e iterações do solver ficam no DEFAULT de cada engine** (spec §2.1
  proíbe mexer). Consequência: o **tempo-até-sleep é excluído como métrica de comparação**
  (limiares 0,005 / 0,03 / 5-frames medem coisas diferentes) → Torre lida por **colapso**.
- **Warmup descartado:** primeiros **25 passos (~0,5 s sim)** da Chuva não entram na coleta.
- Outras normalizações (funil→caixa fechada, restituição, atrito único) em
  `assignments/SCENE_SPEC.md` → seção *"Desvios do spec"*.

---

## 4. Mapa de arquivos

| Arquivo | O que é |
|---|---|
| `README.md` | **Guia de leitura dos resultados — comece por aqui** (índice + dicionário das colunas dos CSVs). |
| `RESULTS.md` | Média Final (M±σ) de **todas** as configs, n=15. Gerado por `aggregate.js`. |
| `COMPARISON-n10-vs-n15.md` | Novo (n=15) × antigo (n=10) lado a lado — validação de estabilidade. |
| `METRICS-e-exclusoes.md` | **Este** guia. |
| `ANALYSIS-comparativo.md` · `AUDITORIA.md` | Análise crítica e auto-auditoria — **do piloto n=10 (histórico)**; conclusões mantidas, mas os números finais estão no `RESULTS.md`. |
| `aggregate_stats.py` / `aggregate.js` | Agregador (Python original / porta Node — mesma metodologia). |
| `_arquivo-2026-07-01-n10/`, `_arquivo-obsoleto/` | Datasets fora da comparação (§3.5). |
