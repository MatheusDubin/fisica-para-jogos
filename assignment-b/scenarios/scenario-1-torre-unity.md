# Cenário 1 — A Torre: Unity

> Implementação específica para Unity.
> Base conceitual: `scenario-1-torre-generic.md`
> Versão de referência: Unity 6 LTS (PhysX path — GameObject workflow)

---

## Contexto do Grau A

O Grau A identificou que Unity usa **PhysX 4.x** no path clássico de GameObjects. A biblioteca PhysX 4 *oferece* o solver **TGS (Temporal Gauss-Seidel)** (GTC 2019, NVIDIA), mas **o default do Unity NÃO é o TGS** — a integração usa **PGS (Projected Gauss-Seidel)** por padrão. Confirmado empiricamente no Grau B: `ProjectSettings/DynamicsManager.asset → m_SolverType: 0` (`0` = PGS default; `1` = TGS, opt-in via Project Settings). *(Correção da suposição original, que assumia TGS.)* O que isso significa para a Torre:

- **Warm starting ativo:** o PhysX reutiliza a solução de impulsos do frame anterior como ponto de partida, o que acelera convergência em pilhas estáveis (vale para PGS e TGS).
- **PGS (default) faz a correção posicional via Baumgarte só ao final do passo** — não reatualiza contatos/Jacobianos durante as iterações como o TGS faria. Em pilhas altas isso sub-resolve a força de sustentação da base e acumula erro. **Resultado empírico (Grau B): a Torre N=100 no Unity/PGS COLAPSA (pancaking)** — não é só "jitter residual". O TGS (opt-in) sustenta apenas ~1 degrau de N a mais (colapsa em N=20 vs N=15 do PGS); nenhum dos dois sustenta pilha alta com iterações padrão.
- **Sem equivalente ao Shock Propagation:** o Chaos (Unreal) tem um mecanismo explícito para distribuir impulsos da base ao topo da pilha; o PhysX não. Isso deve aparecer como diferença no tempo até sleep e na presença de jitter — mesmo com TGS.
- **Sleep threshold padrão:** `0.005 m/s` linear e `0.005 rad/s` angular — relativamente sensível. Documentar o valor exato encontrado nas Project Settings antes de rodar.
- **`ArticulationBody`** seria mais estável para cadeias (descoberto no Grau A), mas não usaremos — o assignment pede `Rigidbody` padrão, que é o que PhysX processa via constraint solver convencional.
- **Dois sistemas de física coexistem no Unity:** o Grau A documentou que Unity tem o path clássico (PhysX + `Rigidbody` + `GameObject`) e o path DOTS (`com.unity.physics` + `Entity` + ECS). Estamos usando o **path clássico** — o padrão ao criar um projeto Unity novo, sem instalação de pacotes adicionais. O path DOTS seria muito mais estável em pilhas altas (solver stateless + paralelismo total), mas exigiria reescrever o projeto em ECS — um paradigma completamente diferente. A comparação justa com Unreal e Godot é com o que cada engine entrega por padrão. Ao cruzar os dados, se a Torre Unity apresentar mais jitter que as outras engines, parte da explicação é arquitetural: não estamos usando o melhor solver disponível na plataforma, mas o solver que a maioria dos desenvolvedores usa.

---

## Configuração do Projeto

- [ ] Fixed Timestep: `Edit > Project Settings > Time > Fixed Timestep = 0.02`
- [ ] Physics Iterations: `Edit > Project Settings > Physics > Default Solver Iterations` — documentar o valor padrão (não alterar)
- [ ] Sleep Threshold: `Edit > Project Settings > Physics > Sleep Threshold` — documentar valor padrão
- [ ] Gravity: confirmar `-9.81` no eixo Y

---

## Implementation locks (cross-engine)

> Decisões de implementação fixadas durante a implementação Godot. Replicar
> exatamente para que os números sejam comparáveis 1-pra-1 entre engines.
> Estes não são valores do spec — são padronizações de **como** implementar
> dentro da liberdade que o spec deixa.

| Item | Valor | Razão |
|---|---|---|
| **Arena** | Caixa fechada: chão 60m × 60m + 4 paredes + teto, altura 150m, espessura 2m | Quando o solver falha em sustentar a pilha, cubos podem ser ejetados a >50 m/s. Sem paredes, escapam do chão e nunca dormem (timeout sem informação). Com paredes, são contidos e a métrica continua válida. |
| **Material das paredes** | Mesmo dos cubos (friction 0.5, bounce 0.0) | Sem energia adicional vindo das paredes. |
| **Espaçamento entre cubos** | **1.0** (faces tocando exatamente, sem gap) | "Perfeitamente empilhadas" per spec. Sem gap. |
| **Offset inicial** | base do cubo mais baixo coincide com o topo do chão | Sem flutuação inicial. Com cubo de 1m e centro em y=0.5, chão em y=0. |
| **Spawn** | Em código (não na cena via prefab manual). 100 cubos em coluna vertical centrada em (0, 0). | Reprodutibilidade. |
| **Sleep check** | Em `FixedUpdate`, iterando todos os Rigidbodies | Sincronizado com passo de física. |
| **Recursos compartilhados** | 1 mesh + 1 collider + 1 PhysicMaterial reusados em todos os cubos | Reduz custo de instanciação. |

### CSV de saída (formato cross-engine)

Cabeçalho exato (matches `results/godot/torre-N100/torre_godot.csv`):

```
run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep
```

Adicionalmente, gravar um debug CSV per-physics-frame para análise de
colapso vs jitter:

```
run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y
```

Onde `top_y` é a posição Y do cubo mais alto (detecta colapso) e `max_v` é
a maior velocidade observada (detecta jitter).

---

## API de Sleep State

Como detectar que um `Rigidbody` dormiu:

```csharp
Rigidbody rb = GetComponent<Rigidbody>();
bool dormindo = rb.IsSleeping();
```

Para acordar manualmente (usar apenas se precisar resetar o teste):
```csharp
rb.WakeUp();
```

---

## Script de Benchmark — TowerBenchmark.cs

> A ser implementado. Estrutura esperada:

```csharp
// TowerBenchmark.cs
// Responsabilidades:
// 1. Instanciar NUM_CUBOS caixas empilhadas verticalmente
// 2. Iniciar timer no FixedUpdate
// 3. A cada FixedUpdate, checar se todos os Rigidbodies estão sleeping
// 4. Ao detectar sleep total, registrar tempo e gravar em CSV
// 5. Reiniciar cena automaticamente para a próxima run (até 10 runs)
// 6. Após 10 runs: calcular média, desvio padrão, média filtrada e exibir/gravar resultado final
```

**Parâmetros do Inspector:**
- `int numCubos = 100`
- `float cubeSide = 1f`
- `float spawnOffset = 0.5f`
- `int totalRuns = 10`
- `string outputPath = "Assets/Data/torre_unity.csv"`

---

## Profiler / Métricas

Para o Cenário 1, a métrica principal é **tempo até sleep** — não requer Profiler API de física.

Para observação de jitter, usar:
- `Window > Analysis > Profiler` durante play mode
- Track `Physics` para ver o custo do passo de física durante a simulação da torre

---

## Resultado Esperado

> A preencher após execução.

| # | Tempo até Sleep (s) | Jitter? | Colapso? |
|---|---|---|---|
| 1 | | | |
| ... | | | |
| **Média Final** | | | |

---

## Notas

> Registrar aqui observações durante a implementação e execução.
