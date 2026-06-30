# Cenário 2 — Chuva de Corpos: Unreal Engine

> Implementação específica para Unreal Engine.
> Base conceitual: `scenario-2-chuva-generic.md`
> Versão de referência: UE 5.x LTS

---

## Contexto do Grau A

O Grau A identificou que Unreal usa **Chaos Physics** com **islands resolvidos em paralelo**, **Async Physics Tick** (UE 5.4+) e **double precision (LWC)**. O que isso significa para a Chuva:

- **Async Physics Tick — a diferença mais crítica:** Chaos roda a física em thread dedicada com timestep fixo, desacoplado do game thread. O FPS **não cai** mesmo quando Physics Step Time excede 20ms — o jogo continua renderizando enquanto a física processa em paralelo. Isso torna a comparação direta de FPS entre Unreal e as outras engines **enganosa**: um FPS alto no Unreal não significa física mais rápida, significa que o rendering não espera pela física. **Medir e reportar os dois separadamente é obrigatório** para esta engine.
- **Islands em paralelo:** o solver do Chaos agrupa corpos em contact islands e resolve cada island numa thread separada. Com 10k corpos dentro de um funil, eventualmente formam uma grande island — o benefício do paralelismo diminui conforme os contatos criam dependências. Monitorar se o Step Time escala sub-linearmente ou linearmente com as variações.
- **LWC double precision:** o Grau A documentou que Chaos usa double-float para coordenadas (Large World Coordinates). Isso eleva o custo baseline por objeto comparado ao PhysX (single precision) e Jolt (single precision por padrão). Os valores absolutos de Step Time do Unreal podem ser maiores não por ineficiência algorítmica, mas por este custo arquitetural — considerar ao interpretar.
- **Sem GPU para rigid bodies:** confirmado no Grau A. GPU é usada apenas em Niagara Fluids. Física rígida é CPU.
- **BVH broadphase do Chaos:** diferente do SAP/MBP do PhysX. Verificar durante implementação se há configuração de broadphase exposta nas Project Settings.

---

## Configuração do Projeto

- [ ] Fixed Timestep: configurar via `Async Physics Tick` com `Fixed Tick Interval = 0.02s` (UE 5.4+)
- [ ] V-Sync: desativar via `r.VSync 0` no console
- [ ] Frame Rate Cap: `t.MaxFPS 0` no console
- [ ] Gravity: `-980 cm/s²` (equivale a -9.81 m/s² na escala Unreal)
- [ ] Atenção: **toda geometria deve estar na escala de centímetros** — esferas de raio 50cm (= 0.5m)

---

## Implementation locks (cross-engine)

> Decisões de implementação fixadas durante a implementação Godot. Replicar
> exatamente para que os números sejam comparáveis 1-pra-1 entre engines.
> **Lembrete:** Unreal em cm — multiplicar todos os valores em metros por 100.

| Item | Valor (m) | Valor (cm — Unreal) | Razão |
|---|---|---|---|
| **Container** | Caixa fechada 35 × 80 × 35, espessura 2 | 3500 × 8000 × 3500, espessura 200 | Volume suficiente para 10k esferas + altura para queda livre. |
| **Aquecimento (warmup)** | **0.5s descartados** | idem | Estabilizar sistema antes de medir. |
| **Janela de coleta** | **10s** | idem | Spec: "primeiros segundos após o spawn". |
| **Spawn** | Grid 3D pré-calculado dentro da caixa | idem | Determinístico, sem overlap. |
| **Spawn jitter** | **±3cm por eixo, RNG seed baseado em run index** | ±3 cm idem (já é cm) | Evita simetria perfeita; reprodutível. |
| **Spawn origem (acima do chão)** | 25 m | 2500 cm | Altura suficiente para acelerar. |
| **Espaçamento no grid** | 1.15 entre centros | 115 entre centros | Evita overlap inicial (diâmetro = 1m / 100cm). |
| **Esfera** | raio 0.5 | raio 50 | Spec. |
| **Material** | friction 0.4, restitution 0.3 | idem | Spec. |
| **Cycling entre runs** | Se UE crashar em N=10k com `OpenLevel`, fazer **in-place destruction + spawn** (mesma técnica usada em Godot/Jolt) | idem | Aprendido em Jolt — provavelmente NÃO necessário em Chaos (que tem async physics), mas se a 2ª run falhar, este é o fallback. |
| **Coleta** | A cada physics tick: capturar Physics Step Time (via Insights ou stat parser) + FPS via `GAverageFPS` | idem | Sincronizado com passo de física. |

### CSV de saída (formato cross-engine)

Cabeçalho exato (matches `results/godot/chuva-tuned-buffer/chuva_godot.csv`):

```
run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras
```

`amostras` = número de samples na janela. Se < 500, simulação rodando abaixo
de 50Hz (esperado em UE 10k? Documentar).

---

## API de Physics Step Time

Em Unreal, a forma recomendada para medir o tempo do passo de física em runtime é via **console stats**:

```
stat physics    ← exibe PhysicsTime no HUD
stat unit       ← exibe Game, Draw e GPU time por frame
```

Para capturar programaticamente em C++:

```cpp
// Via FCsvProfiler (UE 5.0+)
#include "ProfilingDebugging/CsvProfiler.h"

// Ou via IConsoleManager para ler stats em runtime:
// Investigar: UEngine::GetStatValueByName ou similar
// A confirmar durante implementação — ver documentação de Unreal Insights
```

> **Alternativa prática:** usar **Unreal Insights** para capturar traces das 10 runs e extrair o valor `Physics` da track `CPU Usage`.

Em Blueprint, uma aproximação:
```
Event Tick → Get World Delta Seconds → registrar
// Não é Physics Step Time real, mas pode servir como proxy se async physics estiver ativo
```

---

## API de FPS

```cpp
// C++
float fps = 1.0f / FApp::GetDeltaTime();
// ou via GAverageFPS (global)
extern ENGINE_API float GAverageFPS;
```

Em Blueprint: node `Get Frame Rate`.

---

## Script de Benchmark

> A ser implementado. Estrutura esperada:

```
// RainBenchmarkActor (C++ ou Blueprint)
// Responsabilidades:
// 1. Receber NUM_OBJETOS como parâmetro (1000, 5000, 10000)
// 2. SpawnActor para cada esfera com UStaticMeshComponent + SetSimulatePhysics(true)
// 3. Durante janela de coleta (10s), a cada physics tick:
//    - Capturar Physics Step Time (via Insights trace ou stat parser)
//    - Capturar FPS
//    - Armazenar
// 4. Ao fim: calcular média e gravar via FFileHelper::SaveStringToFile
// 5. Reiniciar level: UGameplayStatics::OpenLevel(GetWorld(), CurrentLevelName)
```

---

## Unreal Insights (ferramenta recomendada)

Para capturar Physics Step Time com precisão, usar **Unreal Insights**:

1. Iniciar trace: `Trace.Start <caminho>` no console
2. Rodar o benchmark
3. Parar: `Trace.Stop`
4. Abrir o trace no Unreal Insights Viewer
5. Filtrar por `Physics` na CPU track
6. Exportar valores por frame

---

## Resultado Esperado

> A preencher após execução.

### Variação 1.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

### Variação 5.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

### Variação 10.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

---

## Notas

> Registrar aqui observações durante a implementação e execução.
