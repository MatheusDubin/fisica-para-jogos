# HANDOFF — Godot pronto, próximo: Unity

> **Você é o agente que vai implementar o assignment-B em Unity.** Este
> documento é seu briefing completo. Godot já foi feito (Cenários 1 e 2).
> Sua missão: replicar a mesma metodologia em Unity para que os três engines
> sejam comparáveis no relatório final.

---

## 1. Missão

Implementar os dois cenários do **assignment-B** em **Unity (GameObject + PhysX,
não DOTS)**:

- **Cenário 1 — A Torre:** N=100 cubos perfeitamente empilhados, medir tempo
  até sleep total. 10 runs, timeout 60s.
- **Cenário 2 — A Chuva:** 1.000 / 5.000 / 10.000 esferas em queda dentro de
  caixa fechada. Medir Physics Step Time médio e FPS médio. 10 runs por
  variação.

---

## 2. Leitura obrigatória antes de começar

Nesta ordem:

1. `assignment-b/scenarios/scenario-1-torre-generic.md` — conceitos e variáveis
   de controle obrigatórias (idênticas nas 3 engines)
2. `assignment-b/scenarios/scenario-1-torre-unity.md` — implementação específica
3. `assignment-b/scenarios/scenario-2-chuva-generic.md`
4. `assignment-b/scenarios/scenario-2-chuva-unity.md`
5. `assignment-b/howto/howto-unity.md` — passo-a-passo de Unity (Profiler,
   ProfilerRecorder, sleep detection, CSV)
6. `assignment-b/howto/howto-statistics.md` — metodologia estatística (M±σ)
7. `assignment-b/results/godot/NOTES.md` — diário de bordo da implementação
   Godot (decisões, armadilhas, dados)

---

## 3. Parâmetros INVARIANTES cross-engine

Estes valores estão fixados no spec e foram replicados em Godot. **Replicar
exatamente em Unity:**

### Comum aos dois cenários

| Parâmetro | Valor | Onde no Unity |
|---|---|---|
| Fixed timestep | **0.02s** (50 Hz) | `Edit > Project Settings > Time > Fixed Timestep` |
| Gravidade | `(0, -9.81, 0)` | `Edit > Project Settings > Physics > Gravity` |
| V-Sync | desativado | `QualitySettings.vSyncCount = 0` no código |
| Frame cap | sem cap | `Application.targetFrameRate = -1` no código |
| **Solver Iterations** | **PADRÃO da engine** | **NÃO MEXER** — §2.1 do spec proíbe |
| **Sleep Threshold** | **PADRÃO da engine** | **NÃO MEXER** — §2.1 do spec proíbe |
| Build | Release | `Build Settings > Development Build = OFF` |
| Runs por configuração | **10** | RunManager singleton |

### Cenário 1 — Torre

| Parâmetro | Valor |
|---|---|
| Número de cubos | **100** |
| Dimensões do cubo | 1m × 1m × 1m |
| Massa | 1 kg |
| Atrito estático | 0.5 |
| Atrito dinâmico | 0.4 |
| Bounciness (restituição) | **0.0** ← crítico |
| Friction Combine | Average |
| Bounce Combine | Minimum |
| Espaçamento entre cubos | **1.00 exato** (faces tocando — "perfeitamente empilhadas" por spec) |
| Offset inicial do chão | base do cubo mais baixo toca o chão exatamente (y=0.5 se cubo tem centro em 0.5) |
| **Cena** | **Arena fechada: chão 60×60m + 4 paredes + teto, altura 150m** (ver §6) |
| Timeout por run | **60s** |
| Métrica primária | `tempo_ate_sleep_s` |
| Como gerar | em código (não colocar manualmente na cena) |

### Cenário 2 — Chuva

| Parâmetro | Valor |
|---|---|
| Forma | **Esfera**, raio 0.5m (diâmetro 1m) |
| Massa | 1 kg |
| Atrito | 0.4 |
| Bounciness | **0.3** |
| Variações | **1.000, 5.000, 10.000** |
| Container | caixa fechada 35×80×35m (6 paredes estáticas) |
| Spawn | grid 3D com leve jitter (±3cm) para evitar overlap inicial |
| Aquecimento (warmup) | **0.5s descartados** antes da coleta |
| Janela de coleta | **10s** |
| Métrica primária 1 | Physics Step Time médio (ms) — via `ProfilerRecorder` |
| Métrica primária 2 | FPS médio na janela |

---

## 4. O que Godot já produziu (use como referência de comparação)

> **Estes números NÃO são target.** Unity é uma engine diferente (PhysX vs.
> Jolt), os resultados naturalmente serão diferentes. Use só para sanity-check
> ("Unity me deu 2ms em 10k esferas — isso parece plausível?").

### Torre N=100 (Godot/Jolt, defaults, com arena)

**CANÔNICO:** `results/godot/torre-N100-arena/torre_godot.csv` (10 runs, 0 timeouts):

| Run | Tempo até sleep | Timeout? | Max velocity |
|---|---|---|---|
| 1  | 10.049s | ✓ sleep | 36.30 m/s |
| 2  | 8.977s  | ✓ sleep | 35.61 m/s |
| 3  | 8.637s  | ✓ sleep | 36.62 m/s |
| 4  | 8.677s  | ✓ sleep | 36.43 m/s |
| 5  | 8.696s  | ✓ sleep | 35.93 m/s |
| 6  | 8.917s  | ✓ sleep | 35.54 m/s |
| 7  | 8.577s  | ✓ sleep | 36.33 m/s |
| 8  | 9.417s  | ✓ sleep | 35.68 m/s |
| 9  | 8.217s  | ✓ sleep | 35.47 m/s |
| 10 | 8.717s  | ✓ sleep | 35.58 m/s |

**Estatísticas:** mean = 8.892s, σ = 0.496s (CV 5.6%), filtered mean = 8.827s.

**Síntese Godot/Jolt no Torre:**
- **10/10 colapsam** (max_v ~36 m/s reproduzível em todas as runs). Jolt
  out-of-the-box não sustenta 100 cubos perfeitamente empilhados —
  consistente com a arquitetura sem warm starting de island (contexto Grau A).
- **0 timeouts** — arena contém cubos ejetados, métrica fica limpa.
- **Distribuição apertada** (σ < 0.5s) — o "tempo até pilha de escombros
  parar" é altamente reproduzível em Jolt.
- O dataset histórico `torre-N100/` (sem arena, 5/10 timeouts por rogue
  cubes) fica para o slide de Considerações Críticas — ilustra a importância
  de conter o volume de medição.

**O que esperar de Unity (PhysX)**: PhysX tem warm starting + TGS solver.
Provavelmente N=100 **vai sustentar a pilha**, com tempo até sleep relativamente
curto (~poucos segundos) e jitter mínimo. **Se Unity também colapsar como
o Jolt, isso seria um achado significativo** — sugeriria que N=100 estoura
mesmo o solver com warm starting.

### Chuva (Godot/Jolt)

`results/godot/chuva-default-buffer/chuva_godot.csv` (1k, 5k) +
`results/godot/chuva-tuned-buffer/chuva_godot.csv` (10k):

| N esferas | Step Time médio | FPS médio | Notas |
|---|---|---|---|
| 1.000 | ~2 ms | ~144 | Defaults OK |
| 5.000 | ~10 ms | ~60 | Defaults OK |
| 10.000 | ~32 ms | ~9–46 (instável) | Precisou bumpar buffers internos do Jolt |

**O que esperar de Unity (PhysX):** números na mesma ordem de grandeza. PhysX
e Jolt são ambos impulse-based; a diferença vai estar em onde cada um aloca
o custo. Unity provavelmente NÃO precisa de tuning de buffers para 10k.

---

## 5. Lições aprendidas em Godot (aplique em Unity quando relevante)

### Faça isto

- **Teste a câmera ANTES** de rodar 10× ao longo de minutos. Em Godot
  perdemos uma sessão inteira com a câmera apontando para o vazio.
- **Spawn dos cubos/esferas em código** (não na cena via prefab manual). Isso
  garante posicionamento exato e reprodutibilidade.
- **Sleep detection em FixedUpdate**, não Update — só FixedUpdate roda em
  sincronia com o passo de física.
- **CSV de diagnóstico per-physics-frame** vale ouro. Em Godot gravamos
  `torre_godot_debug.csv` com `asleep, max_v, mean_v, top_y` por frame. Isso
  permite distinguir colapso (top_y caindo) de jitter (max_v oscilando sem
  queda) sem precisar revisitar a simulação.
- **Para a Chuva em 10k**, se você notar crash na 2ª run ou estourar limites
  internos do PhysX, faça **in-place reset** em vez de `SceneManager.LoadScene`:
  destrói os GameObjects das esferas, espera 2 FixedUpdates, spawna o próximo
  batch. Foi isso que destravou Godot em N=10k.
- **Spawn jitter ±3cm com seed determinístico** (não totalmente aleatório) —
  permite reprodutibilidade entre runs sem overlap inicial.

### Não faça isto

- **Não mude solver iterations.** O spec proíbe (§2.1 generic doc). Anote
  o valor padrão para o slide, mas não modifique.
- **Não mude sleep threshold.** Mesma razão.
- **Não introduza gap entre cubos.** Em Godot tentamos 1.02 (gap 2cm) para
  estabilizar, mas o spec exige "perfeitamente empilhadas" — espaçamento 1.00
  exato. Os resultados que apresentamos têm que vir desse setup.
- **Não trate o spec como sugestão.** A primeira versão desta implementação
  Godot teve que ser refeita porque tinha gap + N reduzido. Releia o spec.
- **USE a arena fechada no Torre** (60×60×150m, com paredes e teto) —
  decidimos preventivamente em vez de chão-só. Razão: cubos podem ser
  ejetados se o solver falhar, e queremos métrica válida em todas as engines.
  Detalhes completos em `scenarios/scenario-1-torre-unity.md` → seção
  "Implementation locks (cross-engine)".
- **Não rode em Development Build.** Os números ficam inflados pela telemetria
  do profiler. Sempre Release.

---

## 6. Arena no Torre — decisão metodológica

Spec (§2.2 generic) só prescreve chão estático para o Torre. Em Godot,
no N=100 com defaults, **Jolt ejetou cubos a ~97 m/s e eles voaram para fora
do chão de 60m**. As 5 timeouts vieram daí.

**Decisão atual (vale para todas as 3 engines):** usar **arena fechada
preventivamente**. Razão: queremos comparação 1-pra-1. Se uma engine ejetar
cubos e outra não, a "engine que ejetou" terá métricas falsas (timeouts por
cubos perdidos, não por solver não-convergir). Arena fechada elimina esse
modo de falha em todas as engines simultaneamente.

**Dimensões padronizadas (replicar exatamente):**
- Chão: 60m × 60m, espessura 2m
- Altura da arena: 150m (suficiente para cubos lançados verticalmente)
- 4 paredes laterais + 1 teto, todos `StaticBody3D`/`StaticMeshActor`
- Material das paredes: friction 0.5, bounciness 0.0 (mesmo dos cubos)

Por que **150m** de altura: cubos lançados a 50 m/s pelo solver atingem
~127m antes de cair de volta. 150m garante que não saem pelo topo.

Por que **30m da pilha às paredes**: para uma pilha de 100m de altura
(cubos perfeitamente empilhados em 1m × 100), as paredes estão a 30m do
centro. Distância suficiente para que paredes NÃO influenciem a fase
inicial (cubos não tocam paredes enquanto a pilha está intacta). Só
entram em ação se o solver falhar e ejetar cubos.

**Implicação para o dataset Godot atual:** o `torre-N100/` foi coletado
ANTES desta decisão, com chão-só. Está marcado como histórico no NOTES.md.
Re-run com arena está pendente (ver §10 deste handoff).

Detalhes completos: `scenarios/scenario-1-torre-unity.md` → seção
"Implementation locks (cross-engine)".

---

## 7. Onde gravar os dados

Estrutura espelhando o Godot:

```
assignment-b/results/unity/
├── NOTES.md                            (escreva conforme implementa — diário de bordo)
├── torre-N100/
│   ├── torre_unity.csv                 (run, num_cubos, tempo_ate_sleep_s, timeout, ...)
│   └── torre_unity_debug.csv           (run, t_s, phys_frame, asleep, max_v, mean_v, top_y)
└── chuva-default/                      (ou chuva-tuned/ se você precisar tunar PhysX)
    └── chuva_unity.csv                 (run, variacao, physics_step_ms_medio, ..., fps_medio)
```

**Formato CSV idêntico ao Godot** (mesmas colunas, mesma ordem). Isso permite
gerar o relatório comparativo final com scripts simples.

### Header dos CSVs (copiar e colar)

**`torre_unity.csv`:**
```
run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep
```

**`torre_unity_debug.csv`:**
```
run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y
```

**`chuva_unity.csv`:**
```
run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras
```

---

## 8. Implementações Godot como referência

Os scripts Godot **não estão dentro de `fisica-para-jogos/`**. Eles vivem em
um diretório irmão: **`C:\Users\mathe\Documents\physics\Projects\physics\`**
(o projeto Godot abre essa pasta). Caminhos:

- `Projects/physics/scripts/run_manager.gd` — singleton autoload, gerencia
  contadores de run, escreve CSV, controla quando recarregar cena
- `Projects/physics/scripts/tower_benchmark.gd` — instancia cubos, detecta
  sleep, registra métricas, recarrega cena
- `Projects/physics/scripts/rain_benchmark.gd` — instancia esferas, mede
  Physics Step Time + FPS na janela, **faz in-place reset** entre runs (não
  recarrega cena) para evitar crash em 10k
- `Projects/physics/project.godot` — Project Settings (timestep, V-Sync,
  buffer tunings do Jolt)

Use **apenas para inspiração estrutural** — NÃO copie 1-pra-1. Unity tem APIs
diferentes. O `howto-unity.md` já tem snippets em C# para todas as operações
equivalentes (ProfilerRecorder, IsSleeping, RunManager singleton).

Para o projeto Unity, sugere-se criar um diretório irmão também:
`C:\Users\mathe\Documents\physics\Projects\unity\` (ou nome equivalente).
Manter o repo `fisica-para-jogos/` para spec, results e handoffs.

---

## 9. Coisas que talvez você queira pensar (não obrigatório)

- **Built-in vs URP**: tanto faz para o benchmark de física. Built-in tem
  menos overhead de renderização, mas a métrica é Physics Step Time
  isoladamente, então não muda muito.
- **`Rigidbody.IsSleeping()` vs evento de sleep**: usar polling em FixedUpdate
  é mais simples e suficiente. Não há `OnSleep` nativo no Unity.
- **ProfilerRecorder name**: o spec/howto sugere `"Physics.Processing"`. Se
  não funcionar, abra o Profiler durante play mode na track Physics e use o
  nome exato que aparece. Alternativas conhecidas: `"Physics.Simulate"`,
  `"PhysicsManager.FixedUpdate"`.
- **Scene reload vs in-place**: Unity normalmente lida bem com
  `SceneManager.LoadScene` mesmo em 10k. Comece com isso; só vá para in-place
  se observar crash ou comportamento errado na 2ª+ run.

---

## 10. Quando estiver pronto

1. Confira que `assignment-b/results/unity/NOTES.md` está escrito
2. Confira que os CSVs estão no formato esperado (mesmas colunas que Godot)
3. Atualize este HANDOFF com uma seção "Status Unity" no final, listando
   sessões executadas e resultados (como o NOTES.md do Godot tem)
4. Próximo agente vai ler isto + os 3 NOTES.md (godot, unity) para fazer
   Unreal e depois o relatório comparativo

---

## Apêndice — Status Godot (snapshot)

| Item | Status |
|---|---|
| Torre N=100 (sem paredes) | ✅ Histórico — `results/godot/torre-N100/`. Não usar no comparativo. |
| **Torre N=100 (com arena)** | ✅ **CANÔNICO** — `results/godot/torre-N100-arena/`. 10 runs, 0 timeouts, mean 8.89s, σ 0.50s. |
| Torre N=10 / N=25 | ✅ Exploratório (não canônico) — `results/godot/torre-N10|N25/` |
| Chuva 1.000 | ✅ Coletado |
| Chuva 5.000 | ✅ Coletado |
| Chuva 10.000 | ✅ Coletado (buffers Jolt tunados) |
| Bonus stress-test 5→50 | ❌ Iniciado, interrompido em N=15 (colapso visual confirmou hipótese) |
| Slides Considerações Críticas | ⏳ Pendente — depois das 3 engines |
| Relatório final comparativo | ⏳ Pendente — depois das 3 engines |
