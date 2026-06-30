# Unity — Notas de Metodologia e Decisões

> Diário de bordo da implementação Unity. Espelha o `results/godot/NOTES.md`
> para que os três engines sejam comparáveis 1-pra-1 no relatório final.

---

## 1. Ambiente

| Item | Valor |
|---|---|
| Engine | Unity **6000.5.1f1** (Unity 6 LTS) |
| Render Pipeline | URP (template 3D URP) |
| Physics Engine | **PhysX** (path clássico GameObject + Rigidbody — não DOTS) |
| Linguagem | C# |
| Path de física | GameObject + Rigidbody (o padrão; sem `com.unity.physics`) |

### Project Settings (verificados em `ProjectSettings/*.asset`)

| Setting | Valor | Justificativa |
|---|---|---|
| `Time > Fixed Timestep` | **0.02** (50 Hz) | Spec do assignment |
| `Time > Maximum Allowed Timestep` | 0.33333 | padrão (não alterado) |
| `Physics > Gravity` | (0, **-9.81**, 0) | Spec |
| `Physics > Default Solver Iterations` | **6** | **PADRÃO — não alterado** (§2.1 do spec proíbe) |
| `Physics > Default Solver Velocity Iterations` | **1** | **PADRÃO — não alterado** |
| `Physics > Sleep Threshold` | **0.005** | **PADRÃO — não alterado** |
| `Physics > Default Contact Offset` | **0.01** | padrão |
| V-Sync | desativado em código (`QualitySettings.vSyncCount = 0`) | Spec |
| Frame cap | removido em código (`Application.targetFrameRate = -1`) | Spec |

### ⚠️ Achado importante: o solver padrão é **PGS**, não TGS

Os scenario docs (escritos a partir da pesquisa do Grau A) assumem que o Unity
usa o solver **TGS (Temporal Gauss-Seidel)** com warm starting. Mas o valor real
encontrado em `DynamicsManager.asset` é:

```
m_SolverType: 0   # 0 = Projected Gauss-Seidel (PGS, default), 1 = Temporal Gauss-Seidel (TGS)
```

Ou seja, **out-of-the-box o Unity 6 usa PGS**, não TGS. O TGS existe (PhysX 4.x
suporta), mas **não é o default** — precisaria mudar `Solver Type` para
"Temporal Gauss Seidel" nas Project Settings, o que seria mexer numa configuração
do solver (proibido pelo §2.1). **Mantivemos PGS (default).**

Implicação para o relatório: a hipótese "Unity sustenta a pilha melhor que o Jolt
por causa do warm starting do TGS" precisa ser revisada — o default é PGS. Se a
Torre Unity colapsar de forma parecida com o Jolt, isso é consistente com PGS-default.
Documentar essa diferença entre a expectativa (TGS) e a realidade (PGS default).

---

## 2. Arquitetura da implementação

Tudo gerado **em código** (mesma filosofia do Godot — zero setup manual de cena,
zero ligação de Inspector), em `Projects/physics-unity/Assets/Benchmark/`:

| Arquivo | Papel |
|---|---|
| `BenchmarkCommon.cs` | Resolução de cenário/saída, escrita de CSV, settings globais, viewer (câmera+luz) |
| `BenchmarkBootstrap.cs` | `[RuntimeInitializeOnLoadMethod]` — cria o harness automaticamente ao dar Play |
| `TowerBenchmark.cs` | Cenário 1 (arena + 100 cubos, sleep detection, debug CSV) |
| `RainBenchmark.cs` | Cenário 2 (caixa + esferas, ProfilerRecorder + FPS, reset in-place) |
| `Editor/BenchmarkMenu.cs` | Menu **Benchmark** numerado (1..5) + itens `Archived -` |
| `Editor/BenchmarkBuild.cs` | Build headless de player standalone (batchmode) |

### Como rodar — ordem do dataset final (30 ciclos cada)

O menu **Benchmark** está numerado na ordem de coleta. **30 ciclos** por
configuração (excede o mínimo de 10 do spec; alinhado ao volume do Godot).

> ⚠️ O **Solver Type** (Project Settings > Physics) não tem API de runtime.
> Itens 1–3 rodam em **PGS** (default); 4–5 exigem trocar para **TGS** antes;
> depois volte para **PGS**.

**Fase A — Solver Type = Projected Gauss Seidel:**
1. **Torre N=100 PGS default** → `torre-N100-arena/`
2. **Torre SWEEP PGS 10..30** → `torre-sweep-default/`
3. **Chuva 1k 5k 10k** → `chuva-default/`

**Fase B — trocar para Temporal Gauss Seidel:**
4. **Torre N=100 TGS** → `torre-N100-tgs/`
5. **Torre SWEEP TGS 10..30** → `torre-sweep-tgs/`

Depois do 5, **volte o Solver Type para Projected Gauss Seidel.**

Itens **Archived -** são exploratórios já analisados (não rodar de novo; os
dados deles já estão no repo). O harness se auto-instancia, roda tudo e para o
Play sozinho. CSVs vão para `assignment-b/results/unity/` (caminho relativo).

**Headless (batchmode), opcional:**
```
# build
Unity.exe -batchmode -quit -projectPath <proj> \
  -executeMethod Benchmark.EditorTools.BenchmarkBuild.Build
# run (escolhe cenário e pasta de saída por env var)
set BENCH_SCENARIO=tower   &  set BENCH_OUT=<...>/results/unity   &  Build\Benchmark.exe
```

### Decisões travadas (cross-engine, replicadas do Godot)

- **Tempo até sleep** medido em **tempo de simulação** (`n_frames * fixedDeltaTime`),
  não wall-clock — determinístico e fisicamente correto. `phys_frames` também gravado.
- **Reset in-place** entre runs em ambos os cenários (destroi corpos, espera 2
  FixedUpdates, respawn) — evita o crash de scene-reload em N alto visto no Jolt.
- **Recursos compartilhados**: 1 `PhysicsMaterial` reusado; mesh/collider das
  primitivas já são compartilhados pelo Unity.
- **`PhysicsMaterial`** (renomeado de `PhysicMaterial` no Unity 6) e
  `Rigidbody.linearVelocity` (renomeado de `.velocity`).

---

## 3. Cenário 1 — A Torre

### Setup canônico

| Parâmetro | Valor | Conformidade |
|---|---|---|
| Número de cubos | **100** | "quantidade elevada" ✓ |
| Dimensões | 1×1×1 m | ✓ |
| Espaçamento | 1.00 (exato, faces tocando) | "perfeitamente empilhadas" ✓ |
| Offset inicial | 0.5 (base toca o chão) | ✓ |
| Massa | 1 kg | ✓ |
| Atrito | dyn 0.4 / stat 0.5 | igual cross-engine |
| Bounciness | 0.0 | crítico ✓ |
| Friction/Bounce Combine | Average / Minimum | spec |
| Arena | fechada 60×60×150 m (chão + 4 paredes + teto, esp. 2m) | decisão cross-engine |
| Timeout | 60 s | spec |
| Runs (ciclos) | **30** | excede o mínimo de 10 do spec |

### CSVs (pasta `torre-N100-arena/`)

- `torre_unity.csv`: `run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep`
- `torre_unity_debug.csv` (per physics frame): `run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y`

### Duas variantes (DEFAULT vs TUNED) — para a análise crítica

O assignment do professor só obriga controlar **malha primitiva, massa, atrito e
fixed timestep** (não menciona solver iterations nem sleep threshold). O "não
mexer no solver" do §2.1 do nosso doc genérico é uma **decisão metodológica
nossa**, não uma exigência. E o professor pede explicitamente para mostrar "os
limites onde a física começa a quebrar" e responder "qual entregou a pilha mais
estável". Então rodamos **duas variantes** (idênticas nas 3 engines):

| Variante | Mudança | Saída | Resultado |
|---|---|---|---|
| **DEFAULT** | PGS, 6 iter (padrão) | `torre-N100-arena/` | 💥 colapsa (out-of-the-box) |
| **+iter** | PGS, 20 iter | `torre-N100-tuned/` | 💥 ainda colapsa → iterações não são a causa |
| **TGS** | troca o algoritmo p/ TGS (6 iter) | `torre-N100-tgs/` | ✅ esperado sustentar → é a arquitetura do solver |

Menu: **Torre N=100 DEFAULT / +iter / TGS**, e os sweeps. A análise detalhada
(números, mecanismo de pancaking, por que iterações não bastam) está na
seção "Resultados (N=100)" acima.

### Nota metodológica: paredes collision-only

Espelhando o Godot, **só o chão é renderizado**; paredes e teto são
collision-only (sem MeshRenderer). Isso (a) deixa a câmera externa enxergar os
corpos e (b) mantém o conjunto renderizado igual ao do Godot, para o FPS da
Chuva ser comparável (sem custo de renderizar 5 paredes que o Godot não desenha).

### Resultados (N=100) — coletados

| Variante | Solver | Iter | mean (s) | σ (s) | filtered (s) | max_v | timeouts | Veredito |
|---|---|---|---|---|---|---|---|---|
| DEFAULT (`torre-N100-arena`) | PGS | 6 | **9.94** | 0.46 | **9.90** | ~40 | 0/10 | 💥 pancake 10/10 |
| +iter (`torre-N100-tuned`) | PGS | 20 | 10.07 | ~0.60 | — | ~37.6 | 0/10 | 💥 pancake 10/10 |
| TGS (`torre-N100-tgs`) | TGS | 6 | — | — | — | — | — | 💥 colapsa (sweep mostra TGS já cai em N=20, logo N=100 pancake) |

Comparação Godot/Jolt (default): mean 8.89s, σ 0.50, max_v ~36, 0 timeouts, 10/10
colapsam. **Unity-PGS é equivalente: colapsa igual, levemente mais lento (9.9 vs
8.9s) e com ejeção um pouco maior (40 vs 36 m/s).**

### Mecanismo: pancaking, NÃO explosão (corrigido)

O debug por frame mostra `top_y` caindo **monotonicamente** de 99.5m → 1.5m: a
coluna **achata reta para baixo** (não explode lateralmente em t=0). max_v=40m/s
é só a velocidade de queda dos cubos enquanto a coluna se esmaga. Logo é uma
**falha de sustentação de carga**: o contato da base precisa segurar o peso de
~100 cubos e o PGS com poucas iterações sub-resolve essa força a cada passo →
a coluna afunda e colapsa. (Por isso `maxDepenetrationVelocity` não ajuda — não
há ejeção a limitar.)

### Iterações ajudam jitter, NÃO colapso (achado)

6→20 iterações no N=100: **nada muda** (ainda pancake). MAS no N=10 (pilha que já
se sustenta): sleep 9–35s → **2.7s** e max_v 0.5 → **0.12**. Ou seja, mais
iterações = convergência mais rápida e menos jitter para pilhas estáveis, mas
**não movem o limiar de colapso** de pilhas altas. O colapso é estrutural do
algoritmo (PGS linear, sem shock propagation), não falta de iterações.

### TGS: ajuda UM passo, NÃO sustenta pilha alta (achado revisado)

Trocar o **algoritmo do solver** para **TGS (Temporal Gauss-Seidel)** — que
atualiza posições durante as iterações — ajuda, mas só **um degrau de limiar**:

- PGS (6 iter): sustenta ≤10, colapsa a partir de **15**.
- TGS (6 iter): sustenta ≤**15**, colapsa a partir de **20**.

TGS é o solver que o Grau A **assumiu** ser o default do Unity (não é; o default é
PGS). A previsão inicial "TGS sustenta N=100" estava **errada**: com apenas 6
iterações o TGS converge melhor por iteração mas ainda não carrega a carga da base
de uma cadeia de 20+ contatos. **Nenhum dos dois solvers sustenta pilha alta no
nº de iterações padrão** — para segurar N=100 seria preciso TGS + muitas iterações
+ substeps (o que jogos reais não fazem; por isso evitam pilhas rígidas altas).
Achado honesto e mais rico que "TGS resolve tudo".

> Detalhe p/ slide: em N=20 TGS, uma run levou **36s** para resolver — o regime de
> "jitter excessivo" do spec: bem no limiar a pilha nem assenta rápido nem colapsa
> limpo, fica teimando por dezenas de segundos. (Unity não expõe API de runtime p/
> solver type — confirmado no DLL; troca-se em `Project Settings > Physics >
> Solver Type`.)

### Sweep — limiar de colapso (dados reais)

| N | DEFAULT (PGS 6) | +iter (PGS 20) | TGS (6) |
|---|---|---|---|
| 10 | ✅ 0.5 | ✅ 0.12 | ✅ 0.28 |
| 15 | 💥 16.5 | — | ✅ **0.44** |
| 20 | 💥 22 | 💥 23 | 💥 21–26 |
| 30 | — | 💥 27 | 💥 28 |

> Tabela acima é do sweep exploratório (3 runs/N). **Sweep canônico com 10 runs/N**
> (N=10,15,20,25,30, PGS e TGS) → pastas `torre-sweep-default/` e `torre-sweep-tgs/`
> (a coletar). Limiar PGS entre 10–15, TGS entre 15–20. PGS bate com Godot/Jolt
> (colapso visual já em N=15).

---

## 4. Cenário 2 — A Chuva

### Setup canônico

| Parâmetro | Valor | Conformidade |
|---|---|---|
| Forma | Esfera raio 0.5 m | ✓ |
| Massa | 1 kg | ✓ |
| Atrito | 0.4 | spec |
| Bounciness | 0.3 | spec |
| Variações | 1.000 / 5.000 / 10.000 | spec |
| Container | caixa fechada 35×80×35 m (6 paredes) | spec |
| Spawn | grid 3D, spacing 1.15, jitter ±3cm (seed 0xBEEF+run) | determinístico sem overlap |
| Spawn origem y | 25 m | spec lock |
| Warmup | 0.5 s descartados | spec lock |
| Janela | 10 s (wall-clock) | spec lock |
| Runs/variação | **30** | excede o mínimo de 10 do spec |

### Métrica de Physics Step Time — ⚠️ corrigida (ProfilerRecorder não funcionou)

**Tentativa 1 (falhou):** `ProfilerRecorder(ProfilerCategory.Physics,
"Physics.Processing")`. Esse marcador **não existe no Unity 6000.5** — o recorder
vinha `Valid=false` e `LastValue=0`. A run preliminar gravou
`physics_step_ms_medio=0.0000` (FPS e amostras OK, mas step time morto).
`"Physics.Simulate"` / `"PhysicsManager.FixedUpdate"` também não são confiáveis.

**Solução (atual):** stepping manual cronometrado.
- `Physics.simulationMode = SimulationMode.Script` no `Start`.
- A cada FixedUpdate: `Stopwatch` em volta de `Physics.Simulate(fixedDeltaTime)`.
- `stepMs = sw.Elapsed.TotalMilliseconds`.

É o equivalente exato ao `TIME_PHYSICS_PROCESS` do Godot (tempo gasto no passo
de física), **independe de build** (Release ou Editor) e **nunca retorna 0**.
FPS continua via `1/Time.unscaledDeltaTime` lido em Update.

### CSV (pasta `chuva-default/`)

- `chuva_unity.csv`: `run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras`
- `amostras < 500` ⇒ simulação abaixo de 50Hz (spiral of death).

### Resultados

> ⏳ Pendente. A run preliminar (1k, 1 run) teve **step time inválido (0.0)** pelo
> bug do ProfilerRecorder — **descartada**. Re-rodar com a métrica corrigida via
> menu **3. Chuva 1k 5k 10k (30 runs)**. Sanity-check vs Godot: ~2 / ~10 / ~32 ms.

---

## 5. Status (snapshot — antes do dataset final de 30 ciclos)

| Item | Status |
|---|---|
| Scripts (código-driven, Torre + Chuva) | ✅ Implementados e compilando |
| Menu de Editor numerado (1..5) + Archived + build headless | ✅ |
| Project Settings conferidos (timestep, gravity, solver default = **PGS**) | ✅ |
| Métrica de step time da Chuva (Stopwatch + Physics.Simulate) | ✅ Corrigida |
| Paredes collision-only (paridade com Godot) | ✅ |
| **Achados de física** (PGS vs TGS, limiar de colapso, pancaking) | ✅ Documentados |
| Torre N=100 — dados **preliminares** (10 runs PGS, 10 runs +iter) | 🟡 Coletados, a substituir por 30 ciclos |
| Sweeps Torre — **preliminares** (3 runs/N, PGS/+iter/TGS) | 🟡 Coletados, a substituir por 30 ciclos |
| Chuva — run preliminar inválida (step time 0.0) | 🔴 Descartar e re-rodar |
| **Dataset final (30 ciclos, itens 1–5 do menu)** | ⏳ Pendente (rodar depois do Unreal) |
| Vídeos curtos + slides + gráficos comparativos | ⏳ Pendentes |

### Pastas de dados (estado atual)

| Pasta | Conteúdo | Validade |
|---|---|---|
| `torre-N100-arena/` | PGS default, 10 runs | preliminar (→ 30) |
| `torre-N100-tuned/` | PGS +iter20, 10 runs | exploratório (Archived) |
| `torre-N100-tgs/` | (vazio) | a rodar (item 4) |
| `torre-sweep-default/` | PGS, 3 runs, N=10/15/20 | preliminar (→ 30, item 2) |
| `torre-sweep-tgs/` | TGS, 3 runs, N=10/15/20/30 | preliminar (→ 30, item 5) |
| `torre-sweep-tuned/` | +iter20, 3 runs | exploratório (Archived) |
| `torre-sweep/` | 1ª varredura PGS (histórica) | histórico |
| `chuva-default/` | 1 run, step time 0.0 | **inválida — descartar** |

> **Quando voltar (pós-Unreal):** rodar os itens **1→5** do menu (ver §2 "Como
> rodar"), conferir os CSVs, e então gravar os vídeos. Eu (próximo turno) calculo
> as estatísticas filtradas M±σ e preencho as tabelas finais.
