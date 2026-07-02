# Resultados — Benchmark de Motores de Física (Grau B)

> Gerado por `aggregate_stats.py`. Metodologia do enunciado: 10 runs → M e σ (populacional, ÷n) → descartar fora de [M±σ] → Média Final.
> Torre em **segundos** (tempo até sleep). Chuva em **ms** (physics step) e **FPS**.
> Valores de todas as engines já normalizados às mesmas unidades (m, m/s, ms).
>
> **⚠️ Ressalva de método (leia antes de comparar):**
> - **Chuva:** só os datasets **canônicos** (janela sim-time, `amostras=500`, 10 runs) entram aqui. Datasets wall-clock obsoletos foram movidos para `_arquivo-obsoleto/`. O `chuva-optimized` do Unreal é um **exercício** (otimização que piorou), não a coluna principal.
> - **Filtro ±σ em distribuições bimodais / com timeout:** a Média Final **não tem significado físico** quando a distribuição é bimodal (ex.: Godot Chuva 10k FPS, faixa com limite negativo) ou quando há timeouts (que por `howto-statistics.md` são **resultado válido**, não outlier). Nesses casos use **mediana + contagem de timeouts + clusters**, não a Média Final.
> - **Torre — métrica primária é COLAPSO/estabilidade** (`kept%`, tabela abaixo), **não** o tempo-até-sleep: cada engine dorme a um limiar de velocidade diferente (Unity 0.005 · Jolt 0.03 · Chaos por frames), então o tempo-até-sleep **não é comparável 1-pra-1**. Ver `ANALYSIS-comparativo.md`.

## Cenário 1 — A Torre (tempo até repouso total, s)

### Tempo até sleep

| Config | Runs (valores puros, s) | M | σ | Faixa [M±σ] | Descartados | **Média Final** |
|---|---|---|---|---|---|---|
| godot · torre-N100-arena · N=100 | 10.049, 8.977, 8.637, 8.677, 8.696, 8.917, 8.577, 9.417, 8.217, 8.717 | 8.888 | 0.486 | [8.403, 9.374] | 10.049, 9.417, 8.217 (3) | **8.743** |
| godot · torre-sweep · N=10 | 0.483, 11.117, 0.518, 0.518, 0.518, 11.118, 0.518, 0.519, 0.518, 0.518 | 2.635 | 4.242 | [-1.607, 6.876] | 11.117, 11.118 (2) | **0.514** |
| godot · torre-sweep · N=15 | 12.338, 8.698, 10.278, 13.258, 14.118, 9.758, 14.298, 11.639, 11.018, 12.198 | 11.760 | 1.758 | [10.002, 13.518] | 8.698, 14.118, 9.758, 14.298 (4) | **11.788** |
| godot · torre-sweep · N=20 | 11.438, 8.978, 11.758, 8.918, 8.678, 10.298, 10.978, 10.698, 10.278, 9.558 | 10.158 | 1.033 | [9.125, 11.191] | 11.438, 8.978, 11.758, 8.918, 8.678 (5) | **10.362** |
| unity · torre-N100-arena · N=100 | 9.780, 10.920, 9.560, 10.300, 9.780, 9.880, 9.480, 10.320, 9.280, 10.120 | 9.942 | 0.460 | [9.482, 10.402] | 10.920, 9.480, 9.280 (3) | **9.963** |
| unity · torre-N100-tuned · N=100 | 10.100, 11.060, 9.100, 9.700, 9.480, 10.160, 10.380, 10.920, 9.500, 10.320 | 10.072 | 0.602 | [9.470, 10.674] | 11.060, 9.100, 10.920 (3) | **9.949** |
| unity · torre-sweep-default · N=10 | 35.621, 11.060, 9.120 | 18.600 | 12.061 | [6.539, 30.661] | 35.621 (1) | **10.090** |
| unity · torre-sweep-default · N=15 | 8.840, 10.180, 8.640 | 9.220 | 0.684 | [8.536, 9.904] | 10.180 (1) | **8.740** |
| unity · torre-sweep-default · N=20 | 10.980, 11.360, 10.980 | 11.107 | 0.179 | [10.928, 11.286] | 11.360 (1) | **10.980** |
| unity · torre-sweep-tgs · N=10 | 2.440, 4.740, 6.960 | 4.713 | 1.845 | [2.868, 6.559] | 2.440, 6.960 (2) | **4.740** |
| unity · torre-sweep-tgs · N=15 | 11.540, 13.380, 13.340 | 12.753 | 0.858 | [11.895, 13.612] | 11.540 (1) | **13.360** |
| unity · torre-sweep-tgs · N=20 | 19.540, 36.301, 22.980 | 26.274 | 7.228 | [19.046, 33.502] | 36.301 (1) | **21.260** |
| unity · torre-sweep-tgs · N=30 | 12.420 | 12.420 | 0.000 | [12.420, 12.420] | — (0) | **12.420** |
| unity · torre-sweep-tuned · N=10 | 2.840, 2.700, 2.660 | 2.733 | 0.077 | [2.656, 2.811] | 2.840 (1) | **2.680** |
| unity · torre-sweep-tuned · N=20 | 9.400, 9.720, 9.580 | 9.567 | 0.131 | [9.436, 9.698] | 9.400, 9.720 (2) | **9.580** |
| unity · torre-sweep-tuned · N=30 | 11.060, 11.100, 11.320 | 11.160 | 0.114 | [11.046, 11.274] | 11.320 (1) | **11.080** |
| unity · torre-sweep · N=10 | 35.621, 11.060, 9.120, 19.480, 24.260, 25.300, 29.601, 60.001, 60.001, 26.140 | 30.059 | 16.755 | [13.304, 46.813] | 11.060, 9.120, 60.001, 60.001 (4) | **26.734** |
| unity · torre-sweep · N=15 | 8.700, 9.860, 8.980, 8.900, 9.100, 9.080, 9.220, 9.080, 10.260, 9.360 | 9.254 | 0.445 | [8.809, 9.699] | 8.700, 9.860, 10.260 (3) | **9.103** |
| unity · torre-sweep · N=20 | 10.020, 10.720, 10.680, 10.560, 10.660, 10.740, 11.460, 10.400, 10.920, 40.881 | 13.704 | 9.065 | [4.639, 22.770] | 40.881 (1) | **10.685** |
| unreal · torre-sweep · N=10 | 21.112, 21.100, 21.100, 21.100, 21.100, 21.100, 21.100, 21.100, 21.100, 21.100 | 21.101 | 0.004 | [21.098, 21.105] | 21.112 (1) | **21.100** |
| unreal · torre-sweep · N=15 | 28.160, 28.160, 28.160, 28.160, 28.160, 28.160, 28.160, 28.160, 28.160, 28.160 | 28.161 | 0.000 | [28.160, 28.161] | — (0) | **28.161** |
| unreal · torre-sweep · N=20 | 8.260, 8.260, 8.260, 8.260, 8.260, 8.260, 8.260, 8.260, 8.260, 8.260 | 8.260 | 0.000 | [8.260, 8.260] | — (0) | **8.260** |

### Estabilidade da Torre (colapso vs estável)

> `kept%` = altura final do topo ÷ altura esperada se intacta. ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%.

| Config | N | topo esperado (m) | topo final médio (m) | kept% | Veredito |
|---|---|---|---|---|---|
| godot · torre-N100-arena | 100 | 99.5 | 1.69 | 2% | **COLAPSOU** |
| godot · torre-sweep | 10 | 9.5 | 9.47 | 100% | **ESTAVEL** |
| godot · torre-sweep | 15 | 14.5 | 2.09 | 14% | **COLAPSOU** |
| godot · torre-sweep | 20 | 19.5 | 0.76 | 4% | **COLAPSOU** |
| unity · torre-N100-arena | 100 | 99.5 | 1.54 | 2% | **COLAPSOU** |
| unity · torre-N100-tuned | 100 | 99.5 | 1.94 | 2% | **COLAPSOU** |
| unity · torre-sweep-default | 10 | 9.5 | 9.45 | 99% | **ESTAVEL** |
| unity · torre-sweep-default | 15 | 14.5 | 1.08 | 7% | **COLAPSOU** |
| unity · torre-sweep-default | 20 | 19.5 | 0.90 | 5% | **COLAPSOU** |
| unity · torre-sweep-tgs | 10 | 9.5 | 9.49 | 100% | **ESTAVEL** |
| unity · torre-sweep-tgs | 15 | 14.5 | 14.48 | 100% | **ESTAVEL** |
| unity · torre-sweep-tgs | 20 | 19.5 | 0.56 | 3% | **COLAPSOU** |
| unity · torre-sweep-tgs | 30 | 29.5 | 0.69 | 2% | **COLAPSOU** |
| unity · torre-sweep-tuned | 10 | 9.5 | 9.49 | 100% | **ESTAVEL** |
| unity · torre-sweep-tuned | 20 | 19.5 | 1.51 | 8% | **COLAPSOU** |
| unity · torre-sweep-tuned | 30 | 29.5 | 2.17 | 7% | **COLAPSOU** |
| unity · torre-sweep | 10 | 9.5 | 9.45 | 99% | **ESTAVEL** |
| unity · torre-sweep | 15 | 14.5 | 1.11 | 8% | **COLAPSOU** |
| unity · torre-sweep | 20 | 19.5 | 1.64 | 8% | **COLAPSOU** |
| unreal · torre-sweep | 10 | 9.5 | 9.48 | 100% | **ESTAVEL** |
| unreal · torre-sweep | 15 | 14.5 | 5.49 | 38% | **PARCIAL** |
| unreal · torre-sweep | 20 | 19.5 | 1.50 | 8% | **COLAPSOU** |

## Cenário 2 — A Chuva (Physics Step Time e FPS)

### Physics Step Time

| Config | Runs (valores puros, ms) | M | σ | Faixa [M±σ] | Descartados | **Média Final** |
|---|---|---|---|---|---|---|
| godot · chuva-default · 1000 esferas | 2.553, 3.267, 2.818, 2.772, 2.902, 3.021, 3.505, 3.345, 2.667, 2.835 | 2.969 | 0.295 | [2.674, 3.263] | 2.553, 3.267, 3.505, 3.345, 2.667 (5) | **2.870** |
| godot · chuva-default · 5000 esferas | 12.911, 16.863, 15.758, 16.622, 13.631, 14.548, 15.299, 15.454, 17.150, 14.590 | 15.283 | 1.319 | [13.963, 16.602] | 12.911, 16.863, 16.622, 13.631, 17.150 (5) | **15.130** |
| godot · chuva-default · 10000 esferas | 30.109, 34.867, 29.776, 31.823, 33.901, 36.559, 31.127, 34.486, 36.714, 31.003 | 33.037 | 2.461 | [30.575, 35.498] | 30.109, 29.776, 36.559, 36.714 (4) | **32.868** |
| unity · chuva-default · 1000 esferas | 1.415, 1.438, 1.439, 1.430, 1.426, 1.436, 1.456, 1.460, 1.437, 1.429 | 1.437 | 0.012 | [1.424, 1.449] | 1.415, 1.456, 1.460 (3) | **1.434** |
| unity · chuva-default · 5000 esferas | 6.692, 6.909, 6.903, 6.928, 6.891, 6.963, 6.929, 6.905, 6.926, 6.949 | 6.900 | 0.072 | [6.827, 6.972] | 6.692 (1) | **6.923** |
| unity · chuva-default · 10000 esferas | 12.765, 13.298, 13.026, 13.124, 13.009, 13.439, 13.218, 13.104, 13.076, 13.121 | 13.118 | 0.171 | [12.947, 13.289] | 12.765, 13.298, 13.439 (3) | **13.097** |
| unreal · chuva-default · 1000 esferas | 9.917, 9.937, 9.940, 10.012, 10.301, 10.183, 9.995, 10.197, 10.260, 10.530 | 10.127 | 0.191 | [9.936, 10.318] | 9.917, 10.530 (2) | **10.103** |
| unreal · chuva-default · 5000 esferas | 64.441, 65.495, 65.967, 64.433, 66.805, 66.513, 65.800, 65.109, 66.721, 64.577 | 65.586 | 0.879 | [64.707, 66.465] | 64.441, 64.433, 66.805, 66.513, 66.721, 64.577 (6) | **65.593** |
| unreal · chuva-default · 10000 esferas | 143.464, 160.202, 160.999, 149.972, 147.603, 149.345, 144.856, 153.877, 148.554, 149.629 | 150.850 | 5.572 | [145.278, 156.422] | 143.464, 160.202, 160.999, 144.856 (4) | **149.830** |
| unreal · chuva-optimized · 1000 esferas | 10.323, 10.271, 10.296, 10.200, 10.313, 10.299, 10.425, 10.510, 10.598, 10.532 | 10.377 | 0.125 | [10.252, 10.501] | 10.200, 10.510, 10.598, 10.532 (4) | **10.321** |
| unreal · chuva-optimized · 5000 esferas | 73.623, 75.185, 75.357, 74.074, 73.350, 73.231, 73.629, 72.914, 74.452, 74.407 | 74.022 | 0.780 | [73.242, 74.802] | 75.185, 75.357, 73.231, 72.914 (4) | **73.922** |
| unreal · chuva-optimized · 10000 esferas | 164.846, 164.115, 178.155, 189.698, 176.310, 170.857, 170.777, 175.943, 163.885, 170.924 | 172.551 | 7.519 | [165.032, 180.070] | 164.846, 164.115, 189.698, 163.885 (4) | **173.828** |

### FPS médio

| Config | Runs (valores puros, fps) | M | σ | Faixa [M±σ] | Descartados | **Média Final** |
|---|---|---|---|---|---|---|
| godot · chuva-default · 1000 esferas | 1527.4, 1689.9, 1660.5, 1660.7, 1664.2, 1655.8, 1680.7, 1650.3, 1663.5, 1648.1 | 1650.1 | 42.7 | [1607.4, 1692.8] | 1527.4 (1) | **1663.7** |
| godot · chuva-default · 5000 esferas | 462.8, 430.3, 440.3, 429.9, 428.4, 406.4, 410.6, 443.0, 442.8, 432.9 | 432.7 | 15.4 | [417.3, 448.2] | 462.8, 406.4, 410.6 (3) | **435.4** |
| godot · chuva-default · 10000 esferas | 79.9, 10.2, 9.9, 10.2, 37.1, 10.1, 15.1, 10.0, 10.7, 10.0 | 20.3 | 21.4 | [-1.1, 41.7] | 79.9 (1) | **13.7** |
| unity · chuva-default · 1000 esferas | 229.2, 232.8, 224.1, 231.3, 229.2, 230.6, 230.7, 230.2, 230.7, 230.5 | 229.9 | 2.2 | [227.8, 232.1] | 232.8, 224.1 (2) | **230.3** |
| unity · chuva-default · 5000 esferas | 125.0, 123.0, 123.7, 123.8, 124.4, 123.4, 123.6, 122.9, 124.2, 122.3 | 123.6 | 0.8 | [122.9, 124.4] | 125.0, 124.4, 122.3 (3) | **123.5** |
| unity · chuva-default · 10000 esferas | 50.6, 47.3, 49.8, 49.2, 49.6, 36.0, 49.5, 48.3, 48.5, 47.9 | 47.7 | 4.0 | [43.7, 51.7] | 36.0 (1) | **49.0** |
| unreal · chuva-default · 1000 esferas | 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0 | 50.0 | 0.0 | [50.0, 50.0] | 50.0, 50.0, 50.0 (3) | **50.0** |
| unreal · chuva-default · 5000 esferas | 14.6, 14.2, 14.2, 14.6, 14.0, 14.0, 14.2, 14.4, 14.0, 14.5 | 14.3 | 0.2 | [14.0, 14.5] | 14.6, 14.6, 14.0, 14.0, 14.0, 14.5 (6) | **14.2** |
| unreal · chuva-default · 10000 esferas | 6.9, 6.1, 6.1, 6.6, 6.7, 6.7, 6.8, 6.4, 6.6, 6.6 | 6.5 | 0.3 | [6.3, 6.8] | 6.9, 6.1, 6.1 (3) | **6.6** |
| unreal · chuva-optimized · 1000 esferas | 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0, 50.0 | 50.0 | 0.0 | [50.0, 50.0] | 50.0, 50.0 (2) | **50.0** |
| unreal · chuva-optimized · 5000 esferas | 13.1, 12.8, 12.7, 13.0, 13.2, 13.2, 13.1, 13.2, 12.9, 12.9 | 13.0 | 0.2 | [12.9, 13.2] | 12.8, 12.7, 13.2, 13.2, 13.2 (5) | **13.0** |
| unreal · chuva-optimized · 10000 esferas | 6.2, 6.2, 5.7, 5.3, 5.7, 5.9, 5.9, 5.8, 6.2, 5.9 | 5.9 | 0.3 | [5.6, 6.2] | 6.2, 6.2, 5.3, 6.2 (4) | **5.8** |

