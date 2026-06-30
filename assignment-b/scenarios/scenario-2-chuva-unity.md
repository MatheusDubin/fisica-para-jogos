# Cenário 2 — Chuva de Corpos: Unity

> Implementação específica para Unity.
> Base conceitual: `scenario-2-chuva-generic.md`
> Versão de referência: Unity 6 LTS (PhysX path — GameObject workflow)

---

## Contexto do Grau A

O Grau A identificou que Unity usa **PhysX 4.x** no path de GameObjects, com **broadphase SAP/MBP (Multi-Box Pruning)** e **threading parcial**. O que isso significa para a Chuva:

- **SAP/MBP broadphase:** o Multi-Box Pruning do PhysX divide o espaço em regiões e usa Sweep and Prune por eixo. Com 10k objetos em queda livre (alto dinamismo), a BVH precisa ser rebalanceada a cada frame — custo O(n log n) que escala mal com dinamismo extremo. Este é o provável gargalo nas variações 5k e 10k.
- **Threading parcial:** PhysX (no path de GameObjects) roda a simulação em uma worker thread dedicada, mas o solver não é totalmente paralelo — o Gauss-Seidel é inerentemente sequencial por constraint island. Compare com Jolt (Godot), que paralela as islands, e Chaos (Unreal), que também.
- **FPS acoplado ao Physics Step Time:** diferente do Unreal (Async Physics Tick), Unity no path clássico tem um sync point na main thread. Quando Physics Step Time sobe, o FPS cai junto. Isso significa que **FPS é uma métrica útil aqui** — ao contrário do Unreal.
- **Unity Physics DOTS não está em uso:** o Grau A revelou que Unity Physics (DOTS) tem Incremental BVH e paralelismo total via Job System — seria dramaticamente mais rápido neste cenário. Mas o assignment usa GameObjects (PhysX), então estamos testando a path mais comum, não a mais otimizada.
- **Sem aceleração GPU para física:** confirmado no Grau A. Todo o processamento é CPU.
- **Por que não usar Unity Physics (DOTS):** o Grau A documentou que Unity tem dois sistemas de física paralelos que não se comunicam. O path DOTS (`com.unity.physics`) tem Incremental BVH, paralelismo total via Job System + Burst Compiler, e seria dramaticamente mais rápido neste cenário — benchmarks internos da Unity apontam ganhos de 20–100x em cenas com muitos corpos. No entanto, DOTS exige reescrever o projeto inteiro no paradigma ECS, não usa `Rigidbody` nem `GameObject`, e não é o que a maioria dos desenvolvedores Unity usa. Estamos medindo **Unity como engine padrão**, não Unity em seu modo mais otimizado. Ao cruzar os dados: se Unity ficar atrás de Godot (Jolt) na variação de 10k objetos, a explicação não é que Unity é inferior — é que estamos comparando o path padrão do Unity (PhysX, threading parcial) com o padrão do Godot 4.6 (Jolt, multi-threading nativo). É uma diferença de escolha arquitetural de default, não de capacidade máxima da plataforma.

---

## Configuração do Projeto

- [ ] Fixed Timestep: `Edit > Project Settings > Time > Fixed Timestep = 0.02`
- [ ] V-Sync: `Edit > Project Settings > Quality > V Sync Count = Don't Sync`
- [ ] Target Frame Rate: desativar cap (`Application.targetFrameRate = -1` em código)
- [ ] Gravity: `-9.81` eixo Y
- [ ] Physics Layers: criar uma layer para os objetos da chuva para facilitar queries

---

## Implementation locks (cross-engine)

> Decisões de implementação fixadas durante a implementação Godot. Replicar
> exatamente para que os números sejam comparáveis 1-pra-1 entre engines.

| Item | Valor | Razão |
|---|---|---|
| **Container** | Caixa fechada 35m × 80m × 35m, espessura 2m, 6 paredes estáticas | Volume suficiente para 10k esferas + altura para queda livre. |
| **Aquecimento (warmup)** | **0.5s descartados** | Estabilizar sistema antes de medir (sleep state dos primeiros frames é ruído). |
| **Janela de coleta** | **10s** | Spec: "primeiros segundos após o spawn". 10s dá ~500 amostras a 50Hz. |
| **Spawn** | Grid 3D pré-calculado dentro da caixa | Determinístico mas sem overlap inicial. |
| **Spawn jitter** | **±3cm por eixo, RNG seed baseado em run index** (`seed = 0xBEEF + run_idx`) | Evita simetria perfeita (que cria contatos artificialmente sincronizados) sem perder reprodutibilidade. |
| **Spawn origem y** | 25m acima do chão da caixa | Altura suficiente para acelerar antes de tocar paredes/chão. |
| **Espaçamento no grid** | 1.15m entre centros (> diâmetro de 1m) | Evita overlap inicial. |
| **Esfera** | raio 0.5m, mesh + collider compartilhados (1 PhysicMaterial reusado) | Spec + custo reduzido de instanciação. |
| **Material** | friction 0.4, bounciness 0.3 | Spec. |
| **Cycling entre runs** | Se Unity crashar em N=10k com `SceneManager.LoadScene`, fazer **in-place reset** (`Destroy` em todas as esferas, esperar 2 FixedUpdates, spawnar próximo batch) | Aprendido em Godot/Jolt — scene reload em 10k bodies acumula estado interno e crasha na 2ª run. |
| **Coleta** | Em `FixedUpdate`, registrar `ProfilerRecorder.LastValue` (ns → ms) + `1f / Time.unscaledDeltaTime` (FPS) | Sincronizado com passo de física. |

### CSV de saída (formato cross-engine)

Cabeçalho exato (matches `results/godot/chuva-tuned-buffer/chuva_godot.csv`):

```
run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras
```

`amostras` = número de FixedUpdate samples na janela. Se `amostras < 500`,
indica que a simulação está rodando abaixo de 50Hz (spiral of death).

---

## API de Physics Step Time (Profiler)

Unity expõe o custo de física via `ProfilerRecorder`:

```csharp
using Unity.Profiling;

ProfilerRecorder physicsRecorder;

void OnEnable()
{
    // "Physics.Processing" é o marcador interno do PhysX step
    physicsRecorder = ProfilerRecorder.StartNew(ProfilerCategory.Physics, "Physics.Processing");
}

void OnDisable()
{
    physicsRecorder.Dispose();
}

void FixedUpdate()
{
    // Valor em nanosegundos — converter para ms
    float stepTimeMs = physicsRecorder.LastValue / 1_000_000f;
    registrar(stepTimeMs);
}
```

> **Marcadores alternativos a investigar:** `"Physics.Simulate"`, `"PhysicsManager.FixedUpdate"`.
> Confirmar o nome exato via `Profiler Window > Physics track` em modo play.

---

## API de FPS

```csharp
void Update()
{
    float fps = 1f / Time.unscaledDeltaTime;
    registrar_fps(fps);
}
```

---

## Script de Benchmark — RainBenchmark.cs

> A ser implementado. Estrutura esperada:

```csharp
// RainBenchmark.cs
// Responsabilidades:
// 1. Receber NUM_OBJETOS como parâmetro (1000, 5000, 10000)
// 2. Instanciar todas as esferas de uma vez no início
// 3. Durante a janela de coleta (ex: 10s), a cada FixedUpdate:
//    - Ler Physics Step Time via ProfilerRecorder
//    - Ler FPS via Time.unscaledDeltaTime
//    - Armazenar em lista
// 4. Ao fim da janela: calcular média dos valores coletados
// 5. Gravar em CSV: engine, cenário, variação, run, step_time_ms, fps
// 6. Destruir todos os objetos e reiniciar para próxima run
// 7. Após 10 runs: exportar CSV e encerrar (ou exibir resultado)
```

**Variações controladas via serialized field no Inspector:**
```csharp
[SerializeField] int[] variacoes = {1000, 5000, 10000};
[SerializeField] int totalRuns = 10;
[SerializeField] float janelaDeColeta = 10f;
[SerializeField] string outputPath = "Assets/Data/chuva_unity.csv";
```

---

## Funil / Área de Contenção

- Construir com `GameObject` + `MeshCollider` (Static) ou usar 6 `BoxCollider` como paredes
- Dimensionar para acomodar 10k esferas de raio 0.5m empacotadas
- Posicionar abaixo do ponto de spawn

---

## Resultado Esperado

> A preencher após execução.

### Variação 1.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| ... | | | |
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
