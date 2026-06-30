# Godot — Análise estatística por dataset

> Aplicação da metodologia do assignment (§4 de cada scenario doc + howto-statistics):
> 10 runs por configuração → calcular **Média (M)**, **Desvio Padrão (σ)**,
> faixa **[M − σ, M + σ]**, descartar valores fora da faixa, calcular
> **Média Final filtrada**.
>
> **σ aqui é população (divide por N)**, não amostral (N−1). É o que o spec
> implica ao falar de "desvio padrão dos dados coletados" sem qualificar.

---

## Cenário 1 — A Torre

### CANÔNICO: `torre-N100-arena/` (N=100, com arena fechada)

10 runs, 0 timeouts, 10/10 colapsaram (pilha não sustenta, vira pilha de
escombros que assenta).

| # | tempo_ate_sleep (s) | timeout? | max_v (m/s) | Dentro [M ± σ]? |
|---|---|---|---|---|
| 1  | 10.049 | ✗ | 36.30 | ✗ (alto) |
| 2  | 8.977  | ✗ | 35.61 | ✓ |
| 3  | 8.637  | ✗ | 36.62 | ✓ |
| 4  | 8.677  | ✗ | 36.43 | ✓ |
| 5  | 8.696  | ✗ | 35.93 | ✓ |
| 6  | 8.917  | ✗ | 35.54 | ✓ |
| 7  | 8.577  | ✗ | 36.33 | ✓ |
| 8  | 9.417  | ✗ | 35.68 | ✗ (alto) |
| 9  | 8.217  | ✗ | 35.47 | ✗ (baixo) |
| 10 | 8.717  | ✗ | 35.58 | ✓ |

| Métrica | Valor |
|---|---|
| **Mean (M)** | **8.888 s** |
| **σ** | **0.486 s** |
| Coeficiente de variação | 5.5% |
| Faixa [M − σ, M + σ] | [8.403 s, 9.374 s] |
| Excluídos (baixo) | run 9 (8.217 s) |
| Excluídos (alto) | run 1 (10.049 s), run 8 (9.417 s) |
| Runs mantidos | 7 / 10 |
| **Média Final filtrada** | **8.743 s** |

**Findings adicionais:**
- max_v de 35.47–36.62 m/s em **todas** as runs → modo de falha determinístico
  do solver. Cubos ejetados sempre na mesma faixa de energia.
- Jitter? Sim, durante a fase de colapso (~2–6s). Cessa quando a pilha de
  escombros assenta.
- Colapso? Sim, 10/10. Confirma a hipótese (Grau A) de que Jolt sem warm
  starting não converge pilhas perfeitamente empilhadas grandes.

---

### HISTÓRICO: `torre-N100/` (N=100, sem paredes)

10 runs, **5 timeouts** por cubos escaparem do chão.

| # | tempo_ate_sleep (s) | timeout? | max_v (m/s) |
|---|---|---|---|
| 1  | 60.011 | ✓ TIMEOUT | 97.64 |
| 2  | 60.017 | ✓ TIMEOUT | 97.66 |
| 3  | 8.557  | ✗ | 36.05 |
| 4  | 60.017 | ✓ TIMEOUT | 97.61 |
| 5  | 60.017 | ✓ TIMEOUT | 97.63 |
| 6  | 9.356  | ✗ | 34.54 |
| 7  | 9.177  | ✗ | 34.86 |
| 8  | 60.017 | ✓ TIMEOUT | 97.46 |
| 9  | 8.458  | ✗ | 35.60 |
| 10 | 8.497  | ✗ | 34.74 |

**Distribuição bimodal** — filtro M±σ aplicado mecanicamente daria valores
sem significado físico (M=34.4s, σ=25.6s, faixa [8.81, 60.02] mantém quase
tudo). Reporte estratificado:

| Grupo | n | tempo médio | max_v médio |
|---|---|---|---|
| Settled (pilha de escombros) | 5 | **8.809 s** | 35.16 m/s |
| Timeout (cubos perdidos) | 5 | 60.016 s | 97.60 m/s |

**Não usar este dataset no comparativo cross-engine.** Está aqui só para
ilustrar o efeito de paredes vs sem paredes no Jolt (slide de Considerações
Críticas).

---

### EXPLORATÓRIO: `torre-N10/`, `torre-N25/`

Coletados com `CUBE_SPACING = 1.02` (gap 2cm, viola spec "perfeitamente
empilhadas"). Mantidos apenas como evidência qualitativa de que pilhas
pequenas com gap se sustentam em Jolt. **Não computar estatísticas formais
— não conformes ao spec.**

---

## Cenário 2 — A Chuva

### Variação 1.000 esferas (`chuva-default-buffer/`)

| # | step_ms_medio | fps_medio | step Dentro [M ± σ]? | fps Dentro [M ± σ]? |
|---|---|---|---|---|
| 1  | 2.5523 | 1482.88 | ✓ | ✗ (baixo) |
| 2  | 2.5558 | 1565.18 | ✓ | ✓ |
| 3  | 2.4685 | 1567.84 | ✗ (baixo) | ✓ |
| 4  | 2.6554 | 1558.27 | ✓ | ✓ |
| 5  | 2.6190 | 1564.29 | ✓ | ✓ |
| 6  | 2.7667 | 1563.42 | ✓ | ✓ |
| 7  | 2.6520 | 1559.22 | ✓ | ✓ |
| 8  | 2.5147 | 1556.63 | ✓ | ✓ |
| 9  | 2.6429 | 1548.26 | ✓ | ✓ |
| 10 | 2.9429 | 1546.72 | ✗ (alto) | ✓ |

| Métrica | step_time (ms) | fps |
|---|---|---|
| **Mean (M)** | **2.637 ms** | **1551.3 FPS** |
| **σ** | **0.130 ms** | **23.73 FPS** |
| Faixa [M − σ, M + σ] | [2.507, 2.767] | [1527.5, 1575.0] |
| Runs mantidos | 8 / 10 | 9 / 10 |
| **Média Final filtrada** | **2.620 ms** | **1558.87 FPS** |

### Variação 5.000 esferas (`chuva-default-buffer/`)

| # | step_ms_medio | fps_medio | step Dentro? | fps Dentro? |
|---|---|---|---|---|
| 1  | 13.6498 | 415.02 | ✓ | ✗ (alto) |
| 2  | 13.8104 | 346.66 | ✓ | ✓ |
| 3  | 14.2247 | 351.31 | ✗ (alto) | ✓ |
| 4  | 13.3835 | 361.14 | ✓ | ✓ |
| 5  | 12.8702 | 354.98 | ✗ (baixo) | ✓ |
| 6  | 13.0627 | 358.30 | ✓ | ✓ |
| 7  | 13.4004 | 362.43 | ✓ | ✓ |
| 8  | 13.1584 | 367.65 | ✓ | ✓ |
| 9  | 13.2741 | 355.77 | ✓ | ✓ |
| 10 | 15.1149 | 319.66 | ✗ (alto) | ✗ (baixo) |

| Métrica | step_time (ms) | fps |
|---|---|---|
| **Mean (M)** | **13.595 ms** | **359.29 FPS** |
| **σ** | **0.628 ms** | **22.39 FPS** |
| Faixa [M − σ, M + σ] | [12.967, 14.223] | [336.9, 381.7] |
| Runs mantidos | 7 / 10 | 8 / 10 |
| **Média Final filtrada** | **13.391 ms** | **357.28 FPS** |

### Variação 10.000 esferas (`chuva-tuned-buffer/`, com buffers Jolt aumentados)

| # | step_ms_medio | fps_medio | step Dentro? | fps Dentro? |
|---|---|---|---|---|
| 1  | 27.6325 | 9.87  | ✗ (baixo) | ✓ |
| 2  | 29.1387 | 11.02 | ✓ | ✓ |
| 3  | 32.0074 | 10.65 | ✓ | ✓ |
| 4  | 34.4964 | 11.11 | ✓ | ✓ |
| 5  | 38.2961 | 13.32 | ✗ (alto) | ✓ |
| 6  | 28.6932 | 46.29 | ✓ | ✗ (alto) |
| 7  | 31.4260 | 35.08 | ✓ | ✓ |
| 8  | 34.1786 | 9.92  | ✓ | ✓ |
| 9  | 36.4429 | 32.04 | ✗ (alto) | ✓ |
| 10 | 27.4138 | 37.26 | ✗ (baixo) | ✗ (alto) |

| Métrica | step_time (ms) | fps |
|---|---|---|
| **Mean (M)** | **31.973 ms** | **21.66 FPS** |
| **σ** | **3.604 ms** | **13.53 FPS** |
| Faixa [M − σ, M + σ] | [28.368, 35.577] | [8.13, 35.18] |
| Runs mantidos | 6 / 10 | 8 / 10 |
| **Média Final filtrada** | **31.657 ms** | **16.63 FPS** ⚠️ ver caveat |

**⚠️ Caveat — FPS em 10k é bimodal:**

A distribuição de FPS tem dois clusters distintos: runs 1,2,3,4,5,8 entre
**9.87–13.32** FPS; runs 6,7,9,10 entre **32.04–46.29** FPS. Não é ruído
gaussiano — é um sistema bistável. Hipóteses:

1. **Thermal throttling do CPU**: laptop RTX 4050, sustained load por 10 runs
   pode causar boost-clock state-switching entre runs.
2. **Windows process scheduling**: o Godot main thread pode ter sido escalado
   em P-cores vs E-cores em diferentes runs.
3. **Acoplamento step time × frame budget**: quando step time fica próximo de
   1/50s = 20ms, há pressão no orçamento de frame; em 30ms+, o motor pode
   estar pulando renders para alcançar a física.

**Implicação:** o filtered_mean=16.63 FPS **não representa** o comportamento
real do motor. Reportar ambos os clusters separadamente:

| Cluster | n | FPS médio | Step time médio |
|---|---|---|---|
| FPS baixo (rendering-bound) | 6 | 11.0 | 32.6 ms |
| FPS alto (talvez rendering pulou) | 4 | 37.7 | 30.5 ms |

Para o relatório: usar **step_time como métrica primária** (mais estável,
σ/μ = 11%) e tratar FPS como observação secundária com a ressalva da
bimodalidade.

---

## Resumo executivo (para comparação cross-engine)

| Cenário | Variação | Métrica | Mean | σ | Filtered Mean |
|---|---|---|---|---|---|
| Torre | N=100 (arena) | tempo até sleep (s) | 8.888 | 0.486 | **8.743** |
| Chuva | 1.000 | step time (ms) | 2.637 | 0.130 | **2.620** |
| Chuva | 1.000 | FPS | 1551.3 | 23.7 | **1558.87** |
| Chuva | 5.000 | step time (ms) | 13.595 | 0.628 | **13.391** |
| Chuva | 5.000 | FPS | 359.3 | 22.4 | **357.28** |
| Chuva | 10.000 | step time (ms) | 31.973 | 3.604 | **31.657** |
| Chuva | 10.000 | FPS | 21.66 | 13.53 | 16.63 ⚠️ bimodal |

### Escalonamento da Chuva (Godot/Jolt)

| Variação | Step time (filtered) | Razão vs anterior |
|---|---|---|
| 1k | 2.620 ms | — |
| 5k | 13.391 ms | **5.11×** (esperado para 5× objetos com BVH O(n log n)) |
| 10k | 31.657 ms | **2.37×** (sublinear — broadphase reaproveita estrutura?) |

A razão 5k → 10k ser sublinear é interessante. Possíveis explicações:
- Cache locality melhora com densidade
- Jolt's job system paraleliza melhor quando há mais islands
- Algum custo fixo de spawning/init que se amortiza melhor em 10k

**Para investigar quando comparar com Unity/Unreal:** se as outras engines
mostram escalonamento linear puro, isso é um achado positivo para Jolt.

---

## Aplicação a Unity e Unreal

Quando coletar dados em Unity e Unreal, aplicar **mesmo processo de
filtragem**:

1. 10 runs por variação
2. Computar M e σ (população — `pstdev`, não `stdev`)
3. Faixa = [M − σ, M + σ]
4. Excluir valores fora da faixa
5. Reportar M, σ, filtered_mean, e número de runs mantidos
6. Flagar bimodalidade quando observada (não confiar no filtered_mean)

CSVs com mesmo schema permitem rodar a mesma análise via script.
