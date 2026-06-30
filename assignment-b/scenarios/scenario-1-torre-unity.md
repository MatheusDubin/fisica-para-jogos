# Cenário 1 — A Torre: Unity

> Implementação específica para Unity.
> Base conceitual: `scenario-1-torre-generic.md`
> Versão de referência: Unity 6 LTS (PhysX path — GameObject workflow)

---

## Contexto do Grau A

O Grau A identificou que Unity usa **PhysX 4.x** com solver **Gauss-Seidel iterativo** no path clássico de GameObjects. O que isso significa para a Torre:

- **Warm starting ativo:** o PhysX reutiliza a solução de impulsos do frame anterior como ponto de partida, o que acelera convergência em pilhas estáveis. Isso é uma vantagem sobre solvers sem warm starting.
- **Gauss-Seidel em pilhas altas:** o solver resolve restrições sequencialmente (de baixo pra cima ou em ordem arbitrária). Em pilhas com 100+ blocos, o erro acumulado pode não se dissipar completamente em 4–6 iterações padrão — fonte de jitter residual no topo.
- **Sem equivalente ao Shock Propagation:** o Chaos (Unreal) tem um mecanismo explícito para distribuir impulsos da base ao topo da pilha; o PhysX não. Isso deve aparecer como diferença no tempo até sleep e na presença de jitter.
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
