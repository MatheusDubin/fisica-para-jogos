# Comparação — n=10 (antigo) × n=15 (novo)

> Antigo = `_arquivo-2026-07-01-n10/` · Novo = coleta atual. Média Final (M±σ) das configs canônicas.
> Δ = novo − antigo. Objetivo: mostrar que o número final **se manteve** ao subir de 10 → 15 runs (validação de estabilidade).

## Chuva — Physics Step Time (Média Final, ms) — *métrica primária*

| Engine | Esferas | n=10 | n=15 | Δ | Δ% |
|---|---|---|---|---|---|
| godot | 1000 | 2.87 | 2.98 | +0.11 | +3.7% |
| godot | 5000 | 15.13 | 15.22 | +0.09 | +0.6% |
| godot | 10000 | 32.87 | 32.83 | -0.04 | -0.1% |
| unity | 1000 | 1.43 | 1.53 | +0.09 | +6.4% |
| unity | 5000 | 6.92 | 7.15 | +0.23 | +3.3% |
| unity | 10000 | 13.10 | 13.32 | +0.22 | +1.7% |
| unreal | 1000 | 10.10 | 10.39 | +0.29 | +2.8% |
| unreal | 5000 | 65.59 | 67.08 | +1.49 | +2.3% |
| unreal | 10000 | 149.83 | 146.55 | -3.28 | -2.2% |

## Chuva — FPS (Média Final) — ⚠️ *secundária; bimodal em 10k, ler com cautela*

| Engine | Esferas | n=10 | n=15 | Δ |
|---|---|---|---|---|
| godot | 1000 | 1664 | 1695 | +32 |
| godot | 5000 | 435 | 431 | -4 |
| godot | 10000 | 14 | 15 | +2 |
| unity | 1000 | 230 | 233 | +2 |
| unity | 5000 | 124 | 121 | -2 |
| unity | 10000 | 49 | 46 | -3 |
| unreal | 1000 | 50 | 50 | +0 |
| unreal | 5000 | 14 | 14 | -0 |
| unreal | 10000 | 7 | 7 | +0 |

## Torre — Colapso (kept%) — *métrica primária*

| Engine | N | kept% n=10 | kept% n=15 | Veredito (n=15) |
|---|---|---|---|---|
| godot | 10 | 100% | 100% | **ESTAVEL** |
| godot | 15 | 14% | 22% | **COLAPSOU** |
| godot | 20 | 4% | 10% | **COLAPSOU** |
| godot | 100 | 2% | 2% | **COLAPSOU** |
| unity | 10 | 99% | 99% | **ESTAVEL** |
| unity | 15 | 8% | 6% | **COLAPSOU** |
| unity | 20 | 8% | 9% | **COLAPSOU** |
| unity | 100 | 2% | 2% | **COLAPSOU** |
| unreal | 10 | 100% | 100% | **ESTAVEL** |
| unreal | 15 | 38% | 38% | **PARCIAL** |
| unreal | 20 | 8% | 8% | **COLAPSOU** |
| unreal | 100 | — (não coletado) | 2% | **COLAPSOU** |

## Descartes do filtro ±σ (nº de runs excluídos) — Chuva Step Time

| Engine | Esferas | n=10 | n=15 |
|---|---|---|---|
| godot | 1000 | 5 | 5 |
| godot | 5000 | 5 | 5 |
| godot | 10000 | 4 | 5 |
| unity | 1000 | 3 | 5 |
| unity | 5000 | 1 | 1 |
| unity | 10000 | 3 | 1 |
| unreal | 1000 | 2 | 3 |
| unreal | 5000 | 6 | 7 |
| unreal | 10000 | 4 | 4 |

