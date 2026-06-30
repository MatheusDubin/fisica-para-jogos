# Cenário 1 — A Torre: Documento Genérico

> Este documento explica o cenário de forma independente de engine.
> O objetivo é entender os conceitos físicos, o que de fato está sendo testado e como medir corretamente antes de implementar em Unity, Unreal ou Godot.

---

## 1. O Que Este Cenário Testa

A Torre é um **teste de estabilidade do solver de restrições** (constraint solver). Empilhar corpos rígidos em uma pilha vertical coloca o solver em uma situação de máximo estresse estático: cada caixa precisa resolver simultaneamente colisões com a caixa abaixo e com a caixa acima, enquanto a gravidade tenta colapsar a pilha.

Os dois fenômenos que o cenário expõe são:

### 1.1 Jitter (Trepidação)
Jitter é a oscilação residual visível em corpos que deveriam estar em repouso. Ele ocorre quando o solver não consegue convergir para uma solução estável e fica alternando entre dois estados próximos a cada frame.

**Por que acontece:** solvers iterativos (como o Gauss-Seidel projetado, usado em PhysX e Jolt) resolvem restrições em sequência, não simultaneamente. Em uma pilha alta, os erros acumulados de iteração para iteração se propagam para cima da pilha. Se o número de iterações do solver for insuficiente — ou se o passo de tempo for grande demais — os corpos no topo nunca estabilizam completamente.

**Fatores que agravam o jitter:**
- Número de iterações do solver muito baixo (comum: 4–8; mais estável: 10–20)
- Timestep muito grande (>0.02s)
- Coeficiente de restituição (bounciness) > 0
- Massa mal distribuída (caixas com massas muito diferentes na mesma pilha)
- Solver sem *warm starting* (reaproveitamento da solução do frame anterior)

### 1.2 Sleep State (Estado de Repouso)
Quando um corpo rígido tem velocidade linear e angular abaixo de um limiar configurável por N frames consecutivos, o motor o marca como "dormindo" e para de integrá-lo. Isso poupa custo computacional mas exige que o solver tenha primeiro **convergido** o suficiente para que a velocidade residual caia abaixo do limiar.

**O que o tempo até o sleep revela:** a eficiência e a estabilidade do solver. Um solver robusto converge a pilha em poucos segundos; um solver instável pode manter a pilha "acordada" por dezenas de segundos ou nunca dormir (se o jitter mantiver a velocidade acima do limiar).

**Limiares típicos de sleep:**
| Engine | Padrão de velocidade linear | Padrão de velocidade angular |
|---|---|---|
| Unity (PhysX) | 0.005 m/s | 0.005 rad/s |
| Unity Physics (DOTS) | configurável | configurável |
| Unreal (Chaos) | configurável via `SleepThresholdMultiplier` | igual |
| Godot | `linear_damp` + sleep threshold | `angular_damp` + threshold |

---

## 2. Setup do Cenário

### 2.1 Variáveis de Controle (Obrigatório manter iguais nas 3 engines)

| Parâmetro | Valor recomendado | Por quê |
|---|---|---|
| **Fixed Timestep** | 0.02s (50 Hz) | Padrão do assignment; timestep menor aumentaria estabilidade artificialmente |
| **Massa de cada cubo** | 1 kg | Uniforme para isolar variáveis |
| **Dimensões de cada cubo** | 1m × 1m × 1m | Primitiva simples, sem Mesh Collider |
| **Atrito estático** | 0.5 | Típico; igual nas 3 engines |
| **Atrito dinâmico** | 0.4 | Típico; igual nas 3 engines |
| **Restituição (bounciness)** | 0.0 | Crucial: qualquer valor > 0 mantém a pilha "acordada" indefinidamente |
| **Gravidade** | -9.81 m/s² (eixo Y) | Padrão de todas as engines |
| **Iterações do solver** | Padrão da engine | Não modificar — é parte do que estamos comparando |
| **Sleep threshold** | Padrão da engine | Não modificar — é parte do que estamos comparando |

### 2.2 Geometria da Pilha

A pilha deve ser **perfeitamente alinhada** no eixo vertical, sem offset lateral. Qualquer desalinhamento introduz torque parasita e invalida a comparação.

```
[ cubo N   ]   ← topo
[ cubo N-1 ]
[ ...      ]
[ cubo 2   ]
[ cubo 1   ]   ← base (sobre o chão estático)
[__________]   ← plano estático (StaticBody / Static Mesh / floor)
```

**Quantidade de cubos sugerida para exploração:**
- 25 cubos — pilha estável na maioria das engines modernas
- 50 cubos — começa a revelar instabilidades
- 100 cubos — estresse moderado
- 200 cubos — limite onde engines menos robustas colapsam

> Para o assignment, defina uma quantidade fixa (ex: 100 cubos) e use a mesma nas 3 engines. A quantidade exata deve constar no documento de resultados.

### 2.3 Instanciação

Os cubos devem ser criados **em código** (não colocados manualmente na cena) para garantir posicionamento exato. Pseudocódigo:

```
for i in range(NUM_CUBOS):
    cubo = instanciar_cubo_rigido()
    cubo.posicao = Vector3(0, i * ALTURA_CUBO + OFFSET_INICIAL, 0)
    cubo.massa = 1.0
    cubo.restituicao = 0.0
    cubo.atrito = 0.5
    adicionar_a_cena(cubo)

iniciar_timer()
```

---

## 3. O Que Medir e Como

### 3.1 Métrica Principal: Tempo até Sleep Total

**Definição:** o tempo decorrido (em segundos) desde o início da simulação até o momento em que **o último corpo** da pilha entra em sleep state.

**Como detectar sleep em cada engine (conceito genérico):**
- A cada passo de física (FixedUpdate / physics tick), percorrer todos os corpos da pilha e checar se `is_sleeping()` (ou equivalente) é `true` para cada um.
- Quando `sleeping_count == NUM_CUBOS`, registrar o timestamp atual e calcular `tempo_total = timestamp_atual - timestamp_inicio`.

**Pseudocódigo de medição:**
```
timestamp_inicio = tempo_atual()
todos_dormindo = false

a_cada_passo_de_fisica():
    if todos_dormindo: return
    
    count = 0
    for cubo in lista_cubos:
        if cubo.esta_dormindo():
            count += 1
    
    if count == NUM_CUBOS:
        tempo_ate_sleep = tempo_atual() - timestamp_inicio
        registrar(tempo_ate_sleep)
        todos_dormindo = true
```

### 3.2 Métrica Secundária: Presença de Jitter

Jitter é **qualitativo** — deve ser observado e descrito, não apenas medido numericamente. Para o assignment, registrar:

- **Sim/Não:** houve jitter visível durante a simulação?
- **Onde:** nos cubos do topo, do meio ou da base?
- **Duração:** o jitter durou até o sleep ou persistiu mesmo depois?
- **Colapso:** a pilha manteve a forma ou colapsou (física "quebrou")?

Uma forma semi-quantitativa: registrar a velocidade linear máxima observada em qualquer cubo após os primeiros 2 segundos (quando a pilha já deveria estar estabilizando). Valores > 0.1 m/s após 5s indicam jitter significativo.

### 3.3 Observações Opcionais

- **Velocidade angular residual do cubo do topo** ao longo do tempo — revela instabilidade de rotação
- **Número de bodies sleeping por segundo** — mostra a taxa de convergência (slope da curva até sleep total)

---

## 4. Metodologia de Coleta (10 Execuções)

Conforme o assignment, cada medição deve ter 10 execuções isoladas para lidar com variações de OS scheduling.

**Para o Cenário 1, cada "execução" é:**
1. Iniciar a cena do zero (sem estado residual de simulação anterior)
2. Instanciar os cubos
3. Correr a simulação até sleep total (ou timeout de 60s se não convergir)
4. Registrar o `tempo_ate_sleep` em segundos

**Tabela de coleta por execução:**

```
Engine: _____ | Quantidade de cubos: _____
| # | Tempo até Sleep (s) | Jitter? | Colapso? |
|---|---|---|---|
| 1 | | | |
| 2 | | | |
| ... | | | |
| 10 | | | |
| Média | | | |
| Desvio Padrão (σ) | | | |
| Faixa [Média ± σ] | | | |
| Média Final (filtrada) | | | |
```

**Atenção:** se a pilha **nunca** entra em sleep (timeout), registrar o valor como `> 60s` (ou o timeout escolhido) — não descartar. Isso é um resultado importante (indica instabilidade do solver).

---

## 5. Armadilhas Comuns

| Problema | Causa | Solução |
|---|---|---|
| Pilha colapsa imediatamente | Restituição > 0 ou offset inicial errado | Garantir `bounciness = 0` e espaçamento exato |
| Sleep nunca ocorre | Limiar de sleep muito alto ou jitter perpétuo | Verificar configuração de sleep threshold; documentar como resultado |
| Tempo até sleep varia muito entre runs | OS scheduling / GC pauses | Rodar 10x e usar média filtrada (metodologia do assignment) |
| Pilha "flutua" levemente acima do chão | Penetração de colisão mal resolvida | Verificar se o collision margin/contactOffset está correto |
| Resultados diferentes entre builds Debug e Release | Garbage collection, assertions | Sempre medir em builds Release/Shipping |
| Solver iterações diferentes entre engines | Configuração padrão diferente | Documentar as iterações padrão de cada engine na seção de resultados |

---

## 6. O Que Este Cenário Revela Sobre Cada Engine

O tempo até sleep e a presença de jitter são indicadores diretos da **qualidade do constraint solver**:

- **Solver com warm starting:** reaproveita a solução do frame anterior, convergindo mais rápido. PhysX e Jolt têm warm starting; verificar se Unity Physics (DOTS) ativa por padrão.
- **Solver com shock propagation:** distribui impulsos da base para o topo da pilha, resolvendo pilhas altas com muito menos iterações. Chaos Physics (Unreal) implementa `Enable Shock Propagation`. Verificar nas outras engines.
- **Solver com position stabilization:** corrige drift de posição diretamente (Baumgarte stabilization ou pseudo-velocidades). Reduz jitter de longo prazo.
- **Sleep baseado em energia vs velocidade:** alguns solvers usam energia cinética total para decidir sleep; outros usam velocidade linear + angular separadamente. A escolha afeta quão "cedo" os corpos dormem.

Anotar nas considerações críticas dos slides qual engine demonstrou melhor convergência e menor jitter — e cruzar com o que foi aprendido no Grau A sobre a arquitetura do solver de cada uma.

---

## 7. Próximos Passos

Com este documento como base, os próximos arquivos a criar são as implementações específicas por engine:

- `scenario-1-torre-unity.md` — implementação em C# + Rigidbody / Unity Physics (DOTS)
- `scenario-1-torre-unreal.md` — implementação em Blueprint/C++ + Chaos Physics
- `scenario-1-torre-godot.md` — implementação em GDScript + RigidBody3D (Jolt)
