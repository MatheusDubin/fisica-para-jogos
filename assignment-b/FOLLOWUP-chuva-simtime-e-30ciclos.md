# FOLLOW-UP (agentes Godot e Unity): Chuva em tempo-de-simulação + 30 ciclos

> **Origem:** decidido durante a implementação Unreal. A janela de coleta da
> Chuva estava em **wall-clock (10 s reais)**, o que mede estados físicos
> DIFERENTES em cada engine (a engine mais lenta mede queda livre; a mais rápida
> mede uma pilha já formada) → comparação injusta. Correção: janela por
> **tempo de simulação (contagem de passos de física)**, idêntica nas 3 engines.
> **A Torre NÃO muda** (já mede tempo-até-sleep corretamente). Só a Chuva.

---

## Por quê (resumo)

Com física acoplada/limitada ao tempo real, N alto roda abaixo de 50 Hz. Uma
janela de 10 s **wall-clock** então cobre poucos passos simulados:

| N | wall-clock 10 s → sim medido (dados reais Unreal) |
|---|---|
| 1.000 | ~10 s sim (500 amostras) — ok |
| 5.000 | ~3,2 s sim (162 amostras) — parcial |
| 10.000 | ~2,0 s sim (104 amostras) — **queda livre, zero colisão** |

Em 10k, 2 s de simulação = as esferas nem tocaram o chão (cálculo abaixo), então
media-se só broadphase, não o custo de resolver a pilha densa (o "estresse de
colisão" do enunciado). **Godot e Unity têm o mesmo problema nos casos lentos.**

## Cálculo do warmup (por que 6 s bastam) — pior caso 10k

Setup idêntico nas 3 engines: esfera r=0,5 m, spacing 1,15 m, spawn base 25 m,
caixa 35×80×35 m, g=9,81.

- 10k → grade lado ⌈10000^⅓⌉ = 22 → coluna (22−1)×1,15 = **24,15 m**; topo em **49,15 m**.
- Queda livre sincronizada (spacing > diâmetro ⇒ sem colisão na queda): base cai
  24,5 m em **t₁ = √(2·24,5/9,81) = 2,24 s**, batendo a **21,9 m/s**.
- Cascata de pouso: coluna 24,15 m a ~21,9 m/s ⇒ ~1,1 s ⇒ **tudo pousa em ~3,3–3,5 s.**
- Altura da pilha ≈ 5236 m³ / 0,6 / 1225 m² ≈ **7 m**.

⇒ tudo pousa em ~3,5 s. **MAS não precisa de warmup longo:** a janela **sim-time**
(500 passos = 10 s simulados) já cobre queda + pilha por inteiro — é ela que
corrige o bug. O **warmup fica CURTO (25 passos ≈ 0,5 s, igual ao que Godot/Unity
já usam)**, só para descartar o transiente inicial. Warmup longo seria PIOR: cairia
na fase **assentada** (mais barata) e **subestimaria** o custo. Não esperar dormir
(isso é a Torre; pilha dormindo mede MENOS estresse).

---

## Valores a usar (idênticos nas 3 engines)

```
WARMUP_STEPS = 25    # ~0,5 s simulados (descartado; curto de proposito)
WINDOW_STEPS = 500   # 10,0 s simulados (coleta) <- a correcao de verdade
Runs por config = 30
```

`amostras` no CSV passa a ser fixo em **500** (= janela). A lentidão real fica no
**FPS** (10k ~10 FPS) — que continua sendo métrica secundária obrigatória.

---

## Godot — `Projects/physics/scripts/rain_benchmark.gd`

Trocar a lógica de warmup/janela (hoje wall-clock via `Time.get_ticks_msec` e
`RunManager.CHUVA_AQUECIMENTO_S/CHUVA_JANELA_S`) por contagem de `_physics_process`:

```gdscript
# --- constantes (topo do script) ---
const WARMUP_STEPS: int = 25    # ~0,5 s simulados (Godot ja usa 0,5 s de warmup)
const WINDOW_STEPS: int = 500   # 10 s simulados

# --- var nova ---
var _steps: int = 0

# --- em _iniciar_proxima_run(): resetar ---
_steps = 0

# --- substituir _physics_process inteiro por: ---
func _physics_process(_delta: float) -> void:
    if _registrado:
        return
    _steps += 1
    if _aquecendo:
        if _steps >= WARMUP_STEPS:
            _aquecendo = false
            _coletando = true
        return
    if _coletando:
        var step_ms: float = Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000.0
        var fps: float = Engine.get_frames_per_second()
        _step_ms.append(step_ms)
        _fps.append(fps)
        if _step_ms.size() % 100 == 1:
            print("[Chuva %d] coleta sim_t=%.1fs step=%.2fms fps=%.1f n=%d" % [
                _num_objetos, _step_ms.size() * 0.02, step_ms, fps, _step_ms.size()])
        if _step_ms.size() >= WINDOW_STEPS:
            _finalizar()
```

30 ciclos: `RunManager.CHUVA_TOTAL_RUNS = 30` (e `TORRE_TOTAL_RUNS = 30`).

## Unity — `Projects/physics-unity/Assets/Benchmark/RainBenchmark.cs`

Trocar `WARMUP_S`/`WINDOW_S` (wall-clock via `unscaledTime`) por contagem de
`FixedUpdate` (cada FixedUpdate = 1 passo simulado, pois já está em
`SimulationMode.Script` + `Physics.Simulate`):

```csharp
// --- constantes ---
const int WARMUP_STEPS = 25;    // ~0,5 s simulados (Unity ja usa 0,5 s de warmup)
const int WINDOW_STEPS = 500;   // 10 s simulados
int _steps = 0;

// --- em IniciarProximaRun(): ---
_steps = 0;

// --- em FixedUpdate(), logo após calcular stepMs: ---
_steps++;
if (_aquecendo) {
    if (_steps >= WARMUP_STEPS) { _aquecendo = false; _coletando = true; _tColeta = Time.unscaledTimeAsDouble; }
    return;
}
if (_coletando) {
    _stepMs.Add(stepMs);
    if (_stepMs.Count % 100 == 1)
        Debug.Log($"[Chuva {_numObjetos}] coleta sim_t={_stepMs.Count*0.02:F1}s step={stepMs:F2}ms n={_stepMs.Count}");
    if (_stepMs.Count >= WINDOW_STEPS) Finalizar();
}
```

FPS continua amostrado em `Update()` (render fps desacoplado — correto no Unity).
30 ciclos: `bench_rain_runs = 30` e `bench_tower_runs = 30` (menu/PlayerPrefs/env).

---

## Dados a re-coletar (os atuais ficam obsoletos)

| Engine | Pasta obsoleta (wall-clock) | Ação |
|---|---|---|
| Godot | `results/godot/chuva-default-buffer/`, `chuva-tuned-buffer/` | re-rodar (sim-time, 30) |
| Unity | `results/unity/chuva-default/` | re-rodar (sim-time, 30) |
| Unreal | `results/unreal/chuva-default/` | re-rodar (sim-time, 30) |

**Torre:** não muda a metodologia; só subir para **30 ciclos** em todas as engines
(Unreal já roda o sweep N=10..100). Os datasets canônicos da Torre existentes
(Godot `torre-N100-arena`, etc.) podem ser mantidos ou re-rodados a 30 por
consistência.

## ⚠️ Runtime (avisar o dono)

Janela sim-time faz 10k rodar em tempo real longo (Unreal 10k ~90 s/run). 30 runs
× 10k pode levar ~45 min só nessa variação, por engine. O enunciado exige **mínimo
10**; 30 é decisão nossa (excede). Se o tempo apertar, 10 runs é suficiente para
o enunciado — priorizar 10k a 10 e as demais a 30, ou tudo a 10.

## Depois de re-coletar

```
cd assignment-b/results
"<UE>/Engine/Binaries/ThirdParty/Python3/Win64/python.exe" aggregate_stats.py
```
Regera `RESULTS.md` (M, σ, filtro [M±σ], média final) com os novos dados.
