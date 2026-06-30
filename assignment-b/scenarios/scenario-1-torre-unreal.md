# Cenário 1 — A Torre: Unreal Engine

> Implementação específica para Unreal Engine.
> Base conceitual: `scenario-1-torre-generic.md`
> Versão de referência: UE 5.x LTS

---

## Contexto do Grau A

O Grau A identificou que Unreal usa **Chaos Physics** com solver **XPBD (Extended Position-Based Dynamics)**. O que isso significa para a Torre:

- **Shock Propagation:** o Chaos tem um mecanismo explícito que distribui impulsos da base ao topo da pilha a cada passo — feature projetada para exatamente o cenário de pilhas altas. Está disponível como configuração (`Enable Shock Propagation`) no Physics Asset e globalmente nas Project Settings. **Documentar se está ativo por padrão e qual o efeito de ativá-lo vs desativá-lo** — isso é diretamente relevante para comparar com Unity e Godot que não têm equivalente.
- **XPBD vs Gauss-Seidel:** XPBD opera em posições (não velocidades), o que tende a produzir simulações visualmente mais estáveis em repouso, mas com características de convergência diferentes do Gauss-Seidel do PhysX.
- **Physics Thread dedicado:** Chaos roda em thread separada do game thread. O tempo até sleep medido será de wall-clock, mas o custo computacional não bloqueia o game loop — diferença arquitetural relevante para interpretar os dados.
- **Escala em centímetros:** toda a cena deve ser construída em cm (1m = 100cm). Cubos de 1m = 100cm de lado; espaçamento entre cubos = 100cm. Erro comum que invalida a comparação se esquecido.
- **Sleep Threshold:** configurável via `SleepThresholdMultiplier` — documentar o valor padrão antes de rodar.

---

## Configuração do Projeto

- [ ] Fixed Timestep: `Project Settings > Engine > Physics > Max Substep Delta Time` — configurar para 0.02s
- [ ] Alternativamente: `Async Physics Tick` com `Fixed Tick Interval = 0.02`
- [ ] Solver Iterations: `Project Settings > Engine > Physics > Position Iterations / Velocity Iterations` — documentar valores padrão
- [ ] Sleep Threshold: `Project Settings > Engine > Physics > Sleep Threshold Multiplier` — documentar valor padrão
- [ ] Gravity: confirmar `-980` (Unreal usa cm/s², logo -9.81 m/s² = -980 cm/s²) — **atenção à escala**

---

## API de Sleep State

Como detectar sleep em um `UPrimitiveComponent` com física simulada:

```cpp
// C++
UPrimitiveComponent* comp = ...;
bool dormindo = comp->RigidBodyIsAwake() == false;
// ou via delegate: OnComponentSleep
```

Em Blueprint:
- Node: `Is Simulating Physics` + verificar velocidade
- Node: `On Component Sleep` (delegate — acionado quando o body dorme)

---

## Script de Benchmark

> A ser implementado. Estrutura esperada:

```
// TowerBenchmarkActor (Blueprint ou C++)
// Responsabilidades:
// 1. Spawnar NUM_CUBOS StaticMesh Actors com Physics habilitada
// 2. Registrar BeginPlay como t=0
// 3. A cada Tick (ou Event On Component Sleep), checar se todos dormiram
// 4. Ao completar, gravar resultado em arquivo via FFileHelper::SaveStringToFile
// 5. Reiniciar Level automaticamente para a próxima run
```

**Atenção à escala:** Unreal trabalha em centímetros. 1m = 100cm. Ajustar todas as dimensões.

---

## Profiler / Métricas

Para o Cenário 1:
- Abrir `Window > Developer Tools > Session Frontend > Profiler`
- Ou usar `stat physics` no console durante play para monitorar custo do passo de física em paralelo

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
