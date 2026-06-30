# How-To: Cálculo Estatístico das Métricas

> Metodologia definida pelo assignment: 10 runs, média, desvio padrão, filtro ±σ, média final.
> Este documento explica cada passo, mostra como calcular manualmente, e oferece um script Python para automatizar.

---

## A Metodologia em 4 Passos

O assignment define explicitamente:

1. **10 execuções isoladas** → coletar o tempo médio de cada run
2. **Calcular média simples (M) e desvio padrão (σ)** da amostra
3. **Descartar** execuções cujos valores estejam fora de `[M - σ, M + σ]`
4. **Calcular Média Final** apenas com os valores que ficaram dentro da faixa

---

## Por que esse método?

O sistema operacional causa variação nos tempos de execução: processos em background, GC pauses, scheduling da CPU. Uma única medição pode ser um outlier por causa disso, não por comportamento da engine.

O filtro ±σ remove as medições que foram distorcidas pelo ambiente, deixando apenas as medições que representam o comportamento típico da engine.

**Quando descartar faz diferença:**
- Se 8 runs mediram ~5ms e 2 runs mediram ~30ms (spike de GC ou preemptção de OS), a média bruta seria ~11ms — enganosa. Com o filtro, os 2 spikes são descartados e a média final fica ~5ms — o comportamento real.

---

## Passo a Passo Manual

### Exemplo com dados reais

Suponha 10 runs de Physics Step Time (ms) para Unity, variação 1.000 objetos:

```
Run  1:  4.2
Run  2:  4.5
Run  3:  4.1
Run  4: 18.7  ← spike (GC pause)
Run  5:  4.3
Run  6:  4.4
Run  7:  4.2
Run  8:  4.6
Run  9:  4.1
Run 10:  4.3
```

### Passo 1: Média bruta (M)

```
M = (4.2 + 4.5 + 4.1 + 18.7 + 4.3 + 4.4 + 4.2 + 4.6 + 4.1 + 4.3) / 10
M = 57.4 / 10
M = 5.74 ms
```

### Passo 2: Variância e Desvio Padrão (σ)

Variância = média dos quadrados das diferenças em relação à média:

```
Diferenças ao quadrado:
(4.2 - 5.74)² = (-1.54)² = 2.3716
(4.5 - 5.74)² = (-1.24)² = 1.5376
(4.1 - 5.74)² = (-1.64)² = 2.6896
(18.7 - 5.74)² = (12.96)² = 167.9616  ← outlier
(4.3 - 5.74)² = (-1.44)² = 2.0736
(4.4 - 5.74)² = (-1.34)² = 1.7956
(4.2 - 5.74)² = (-1.54)² = 2.3716
(4.6 - 5.74)² = (-1.14)² = 1.2996
(4.1 - 5.74)² = (-1.64)² = 2.6896
(4.3 - 5.74)² = (-1.44)² = 2.0736

Variância = soma / n = 186.8640 / 10 = 18.6864

σ = √18.6864 ≈ 4.32 ms
```

### Passo 3: Aplicar o filtro [M ± σ]

```
Faixa válida = [M - σ, M + σ] = [5.74 - 4.32, 5.74 + 4.32] = [1.42, 10.06]
```

Verificar cada run:

```
Run  1:  4.2 → dentro de [1.42, 10.06] ✅
Run  2:  4.5 ✅
Run  3:  4.1 ✅
Run  4: 18.7 → FORA (18.7 > 10.06) ❌ → DESCARTAR
Run  5:  4.3 ✅
Run  6:  4.4 ✅
Run  7:  4.2 ✅
Run  8:  4.6 ✅
Run  9:  4.1 ✅
Run 10:  4.3 ✅
```

### Passo 4: Média Final (apenas valores dentro da faixa)

```
Valores válidos: 4.2, 4.5, 4.1, 4.3, 4.4, 4.2, 4.6, 4.1, 4.3
Soma = 38.7
N válido = 9

Média Final = 38.7 / 9 ≈ 4.30 ms
```

**A média final (4.30 ms) é bem diferente da bruta (5.74 ms)** — o outlier distorceu significativamente.

---

## Template de Tabela (preencher por run)

```
Engine: _____  |  Cenário: _____  |  Variação: _____ objetos

| # | Valor (ms) | (V - M)² | Dentro [M±σ]? |
|---|---|---|---|
| 1 | | | |
| 2 | | | |
| 3 | | | |
| 4 | | | |
| 5 | | | |
| 6 | | | |
| 7 | | | |
| 8 | | | |
| 9 | | | |
| 10 | | | |
| **Média bruta (M)** | | — | — |
| **Variância** | — | soma/10 | — |
| **Desvio Padrão (σ)** | √variância | — | — |
| **Faixa [M-σ, M+σ]** | [___, ___] | — | — |
| **Runs descartadas** | # ___ | — | — |
| **Média Final** | | — | — |
```

---

## Script Python para Automatizar

Salvar como `calcular_stats.py` na pasta `data/` do repositório.

```python
import csv
import math
import sys
from pathlib import Path

def calcular_media(valores):
    return sum(valores) / len(valores)

def calcular_desvio_padrao(valores, media):
    variancia = sum((v - media) ** 2 for v in valores) / len(valores)
    return math.sqrt(variancia)

def filtrar_e_calcular(valores):
    media = calcular_media(valores)
    sigma = calcular_desvio_padrao(valores, media)
    
    limite_inf = media - sigma
    limite_sup = media + sigma
    
    validos = [v for v in valores if limite_inf <= v <= limite_sup]
    descartados = [v for v in valores if v < limite_inf or v > limite_sup]
    
    media_final = calcular_media(validos) if validos else None
    
    return {
        "media_bruta": media,
        "sigma": sigma,
        "limite_inf": limite_inf,
        "limite_sup": limite_sup,
        "validos": validos,
        "descartados": descartados,
        "n_descartados": len(descartados),
        "media_final": media_final
    }

def processar_csv(caminho_csv):
    """
    Espera CSV com colunas: engine, cenario, variacao, run, physics_step_ms, fps
    """
    grupos = {}
    
    with open(caminho_csv, newline='') as f:
        reader = csv.DictReader(f)
        for linha in reader:
            chave = (linha['engine'], linha['cenario'], linha['variacao'])
            if chave not in grupos:
                grupos[chave] = {'step_ms': [], 'fps': []}
            grupos[chave]['step_ms'].append(float(linha['physics_step_ms']))
            grupos[chave]['fps'].append(float(linha['fps']))
    
    print("=" * 60)
    for (engine, cenario, variacao), dados in grupos.items():
        print(f"\nEngine: {engine} | Cenário: {cenario} | Variação: {variacao}")
        print("-" * 40)
        
        # Physics Step Time
        r = filtrar_e_calcular(dados['step_ms'])
        print(f"Physics Step Time:")
        print(f"  Runs brutas:    {[f'{v:.2f}' for v in dados['step_ms']]}")
        print(f"  Média bruta:    {r['media_bruta']:.3f} ms")
        print(f"  Desvio padrão:  {r['sigma']:.3f} ms")
        print(f"  Faixa válida:   [{r['limite_inf']:.3f}, {r['limite_sup']:.3f}]")
        print(f"  Descartados:    {r['n_descartados']} runs {r['descartados']}")
        print(f"  Média Final:    {r['media_final']:.3f} ms")
        
        # FPS
        r_fps = filtrar_e_calcular(dados['fps'])
        print(f"FPS:")
        print(f"  Média Final:    {r_fps['media_final']:.1f} fps")
    
    print("=" * 60)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python calcular_stats.py <caminho_do_csv>")
        print("Exemplo: python calcular_stats.py data/raw/chuva_unity.csv")
        sys.exit(1)
    
    processar_csv(sys.argv[1])
```

### Como usar

```bash
# Instalar Python (se não tiver): python.org
# Rodar o script:
python calcular_stats.py data/raw/chuva_unity.csv
python calcular_stats.py data/raw/chuva_unreal.csv
python calcular_stats.py data/raw/chuva_godot.csv
```

### Formato esperado do CSV de entrada

```csv
engine,cenario,variacao,run,physics_step_ms,fps
unity,chuva,1000,1,4.2,144
unity,chuva,1000,2,4.5,138
unity,chuva,1000,3,18.7,22
...
```

---

## Casos Especiais

### O que fazer se mais de 5 runs forem descartadas?

Se o filtro ±σ eliminar mais da metade das runs, a variância é muito alta — indica problema no ambiente de teste (outros programas rodando, laptop em modo bateria, etc.). Nesse caso:
1. Documentar o problema
2. Rodar mais 10 execuções em ambiente mais controlado
3. Se persistir, reportar a variância alta como resultado — ela em si é um dado sobre a estabilidade daquela engine naquele cenário

### O que fazer se uma engine nunca entrar em sleep (Cenário 1)?

Registrar como `> 60` segundos (ou o timeout definido). **Não descartar.** É um resultado válido que indica instabilidade do solver.

### O que fazer com o Cenário 1 (Torre)?

Para a Torre, a métrica é **tempo até sleep** (segundos), não Physics Step Time. O mesmo cálculo se aplica — são 10 valores de "tempo até sleep" que passam pela mesma metodologia de média → desvio padrão → filtro → média final.

---

## Excel / Google Sheets (alternativa ao Python)

Se preferir planilha:

1. Colar os 10 valores na coluna A (A1:A10)
2. Média bruta: `=AVERAGE(A1:A10)`
3. Desvio padrão: `=STDEV(A1:A10)` (STDEV usa n-1; STDEVP usa n — o assignment pede n, então usar `=STDEVP(A1:A10)`)
4. Limite inferior: `=B11 - B12` (onde B11 = média, B12 = σ)
5. Limite superior: `=B11 + B12`
6. Filtro (nova coluna B): `=IF(AND(A1>=B$13, A1<=B$14), A1, "")` — deixa vazio se descartado
7. Média final: `=AVERAGE(B1:B10)` (AVERAGE ignora células vazias automaticamente)
