# Estudo Genérico — Benchmark em Unity (Assignment B)

> Fase pré-pesquisa: entender a abordagem conceitual antes de ir à documentação.
> Perguntas abertas ao final de cada seção guiam a fase de pesquisa.

---

## 1. Configuração Base

### Fixed Timestep
No Unity, o passo de simulação física é controlado por `Time.fixedDeltaTime`. Para garantir 50 Hz (0.02s), basta setar via `Project Settings > Time > Fixed Timestep = 0.02` ou por código:

```csharp
Time.fixedDeltaTime = 0.02f;
```

A física do Unity (PhysX path) roda no `FixedUpdate()`, que é chamado nesse intervalo fixo — independente do framerate de renderização.

### Qual pipeline usar no benchmark?
Unity tem dois pipelines de física:
- **PhysX (GameObject):** `Rigidbody` + `Collider` — o mais comum, padrão para projetos não-ECS.
- **Unity Physics (DOTS/ECS):** `PhysicsBody` + `PhysicsShape` — mais performático com muitos corpos, mas exige setup ECS.

Para comparação justa com Unreal e Godot (que não têm ECS), **o benchmark deve usar o pipeline PhysX (GameObject)**. Mas vale registrar nos resultados qual pipeline foi usado.

### Primitivas a usar
- `BoxCollider` (cubo): o mais simples e barato computacionalmente.
- Criar um prefab simples: `GameObject` + `Rigidbody` + `BoxCollider`, massa e atrito padronizados.

---

## 2. Cenário 1 — A Torre

### Abordagem de implementação

Instanciar N cubos empilhados verticalmente via script, usando `Object.Instantiate()` em loop. O espaçamento entre caixas deve ser exato (sem gap ou sobreposição) para forçar o solver a trabalhar desde o início.

```csharp
// Pseudocódigo
for (int i = 0; i < towerHeight; i++) {
    Vector3 pos = new Vector3(0, i * boxHeight + offset, 0);
    Instantiate(boxPrefab, pos, Quaternion.identity);
}
```

### Detecção de repouso (sleep state)

No Unity, um `Rigidbody` entra em sleep quando sua velocidade fica abaixo de `Physics.sleepThreshold` por um número de frames. É possível consultar com `rigidbody.IsSleeping()`.

Para detectar que **a pilha inteira** entrou em repouso, precisamos verificar todos os Rigidbodies da cena:

```csharp
bool allSleeping = rigidbodies.All(rb => rb.IsSleeping());
```

O tempo deve ser medido de `t=0` (instanciação da pilha) até o momento em que `allSleeping == true`.

### O que observar além do tempo
- Jitter visual: oscilação constante nos corpos superiores mesmo após "estabilização"
- Colapso: a pilha desmorona de forma não-realista
- Penetração: corpos "afundando" uns nos outros

### Perguntas abertas para pesquisa
- [ ] Qual é o valor padrão de `Physics.sleepThreshold` no Unity 6? É recomendado ajustá-lo para o benchmark?
- [ ] `IsSleeping()` retorna `true` com jitter residual ou apenas com repouso completo?
- [ ] Existe alguma API para forçar o Unity a reportar o estado de sleep de forma mais granular (por ex., velocidade angular)?
- [ ] Como garantir que todos os Rigidbodies foram criados antes de iniciar o timer (evitar variação por lag de Instantiate)?

---

## 3. Cenário 2 — Chuva de Corpos

### Abordagem de implementação

Spawnar N objetos simultâneos dentro de um "funil" (colliders estáticos formando uma pirâmide invertida) em um único frame ou em rajadas. O funil força colisões entre os corpos e com as paredes, estressando o solver.

```csharp
// Pseudocódigo
void SpawnRain(int count) {
    for (int i = 0; i < count; i++) {
        Vector3 pos = randomPositionInsideFunnel();
        Instantiate(boxPrefab, pos, Random.rotation);
    }
}
```

### O que medir

**Physics Step Time:** tempo que o motor gastou calculando a física em um frame.

No Unity, a forma mais precisa é via **Unity Profiler API**:

```csharp
using UnityEngine.Profiling;
// Profiler.GetCounter não é público nessa granularidade...
// A alternativa é usar o Profiler window ou Recorder API:
var recorder = Recorder.Get("Physics.Processing");
recorder.enabled = true;
// Em FixedUpdate: recorder.elapsedNanoseconds
```

**FPS:** `1.0f / Time.unscaledDeltaTime` (usar unscaled para não ser afetado pelo Time.timeScale).

### Automação das 10 execuções

Cada "execução" deve ser uma run isolada:
1. Spawnar N objetos
2. Aguardar X segundos de simulação (estabilização)
3. Coletar média de Physics Step Time nos últimos Y frames
4. Destruir todos os objetos / recarregar cena
5. Repetir

Isso pode ser feito com um `Coroutine` ou via Editor Script para rodar headless (em batch mode).

### Variações obrigatórias
| Variação | N objetos |
|---|---|
| V1 | 1.000 |
| V2 | 5.000 |
| V3 | 10.000 |

### Perguntas abertas para pesquisa
- [ ] Qual é a API mais confiável para capturar Physics Step Time no Unity 6? `Profiler.Recorder`? `FrameTimingManager`?
- [ ] O `Profiler.Recorder` funciona em builds (não apenas no Editor)? Isso é necessário para rodar em batch mode.
- [ ] Como rodar o Unity em modo headless (sem janela gráfica) para reduzir variáveis externas durante coleta?
- [ ] O Unity tem alguma API de "Physics Statistics" acessível em runtime (tipo `Physics.GetStatistics()`)?
- [ ] Como exportar os dados coletados para CSV automaticamente ao final de cada run?

---

## 4. Ferramentas de Profiling Disponíveis (genérico)

| Ferramenta | Tipo | Granularidade | Disponível em build? |
|---|---|---|---|
| Unity Profiler (Editor) | GUI | Alta (por marcador) | Não (Editor only) |
| `Profiler.Recorder` | API em código | Média (por marcador nomeado) | Sim (com `development build`) |
| `FrameTimingManager` | API em código | Baixa (frame total) | Sim |
| `Time.fixedDeltaTime` vs `Time.fixedUnscaledTime` | API em código | Nenhuma (não mede custo) | Sim |
| Deep Profiler | GUI | Muito alta | Editor only |

### Marcadores relevantes no Profiler (a confirmar na pesquisa)
- `Physics.Processing` — processamento principal do PhysX
- `Physics.FetchResults` — coleta de resultados do solver
- `Physics.UpdateBodies` — atualização de transforms
- `FixedUpdate.PhysicsFixedUpdate` — wrapper do ciclo de física

---

## 5. Armadilhas Conhecidas

- **GC durante o benchmark:** `Instantiate` pode causar alocações. Considerar pooling de objetos para evitar GC spikes nos dados.
- **Vsync:** desativar Vsync (`QualitySettings.vSyncCount = 0`) para não limitar FPS artificialmente.
- **Physics.autoSimulation:** garantir que está `true` (padrão) — se desativado, a física não roda automaticamente.
- **Sleeping automático:** o Unity pode colocar corpos em sleep antes do tempo esperado se `sleepThreshold` estiver alto. Verificar e padronizar antes do teste.
- **Layer matrix de colisão:** garantir que todos os objetos colidem entre si e com o chão/funil.

---

## 6. Perguntas Abertas Gerais (para pesquisa)

- [ ] Qual a diferença de performance entre PhysX (GameObject) e Unity Physics (DOTS) com 10.000 corpos? Vale registrar ambos como variação do benchmark?
- [ ] Existe documentação oficial da Unity sobre como fazer benchmarks de física?
- [ ] Como configurar a "qualidade" da física (solver iterations) de forma padronizada para comparação justa?
- [ ] O Physics.simulationMode padrão é `FixedUpdate` — há alguma vantagem em mudar para `Update` no contexto de benchmark?
