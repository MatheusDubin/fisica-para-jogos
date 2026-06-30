# Cenário 2 — Chuva de Corpos: Documento Genérico

> Este documento explica o cenário de forma independente de engine.
> O objetivo é entender os conceitos físicos, o que de fato está sendo testado, como medir corretamente e como automatizar a coleta antes de implementar em Unity, Unreal ou Godot.

---

## 1. O Que Este Cenário Testa

A Chuva de Corpos é um **teste de estresse do pipeline de física completo** — broadphase, narrowphase e solver — sob carga máxima e simultânea. Diferente do Cenário 1 (que testa estabilidade estática), aqui o que importa é **throughput**: o quanto o motor consegue processar por passo de física quando há um número muito grande de corpos ativos e interagindo ao mesmo tempo.

As variações de 1.000, 5.000 e 10.000 corpos são intencionais: permitem traçar uma **curva de degradação** e identificar em que ponto cada engine começa a quebrar a simulação (Physics Step Time supera o Fixed Timestep, causando spiral of death) ou a descartar frames.

### 1.1 O Pipeline de Física que é Estressado

Cada passo de física executa estas etapas, na ordem:

```
┌──────────────────────────────────────────────────────┐
│  1. Integração   — aplicar forças/gravidade, atualizar velocidades
│  2. Broadphase   — encontrar pares de objetos "potencialmente colidindo"
│  3. Narrowphase  — calcular contatos exatos entre pares candidatos
│  4. Solver       — resolver restrições de contato (impulsos/posição)
│  5. Integração   — atualizar posições finais
└──────────────────────────────────────────────────────┘
```

Com 10.000 corpos em queda livre dentro de um funil, **todas as etapas** são pressionadas ao limite:

- **Broadphase** precisa gerenciar 10k bounding boxes se movendo rapidamente. Estruturas como BVH (Bounding Volume Hierarchy) e SAP (Sweep and Prune) requerem atualizações O(n log n) a cada frame de alta dinamicidade.
- **Narrowphase** precisa avaliar colisões entre pares detectados pela broadphase — potencialmente centenas de milhares de pares dentro do funil.
- **Solver** precisa resolver todas as restrições de contato geradas. Com 10k corpos dentro de um funil, o número de contatos simultâneos pode ser da ordem de 5–30k por passo.

### 1.2 Physics Step Time e a "Spiral of Death"

**Physics Step Time** é o tempo de wall-clock que a CPU gastou executando **um único passo de física** (um fixed timestep de 0.02s).

Se `Physics Step Time > Fixed Timestep (20ms)`, o motor não consegue simular em tempo real. A maioria das engines reage acumulando passos de física pendentes para "recuperar o tempo perdido", o que só piora a situação — chamado de **spiral of death**:

```
Frame 1: physics step demorou 25ms → acumulou 5ms de atraso
Frame 2: precisa rodar 1.25 steps → demora 31ms → acumulou 16ms
Frame 3: precisa rodar 1.8 steps → demora 45ms → ...
→ simulação diverge ou trava
```

Por isso, o objetivo não é apenas medir o Physics Step Time médio, mas identificar **a partir de qual variação** (1k/5k/10k) o Step Time supera o Fixed Timestep de 20ms — isso é o "ponto de quebra" da engine.

### 1.3 FPS e Sua Relação com o Physics Step

**FPS** mede o ritmo de renderização, não diretamente a física. Mas em engines onde física e rendering rodam na mesma thread (ou onde a física bloqueia o game loop), o FPS é um reflexo do custo da física.

- Em engines com **Async Physics** (UE5 Async Physics Tick), o FPS pode se manter alto mesmo com Physics Step alto — porque a física roda numa thread separada.
- Em engines sem física assíncrona, FPS cai diretamente quando a física está sobrecarregada.

Isso significa que **FPS sozinho não é uma métrica confiável** para comparar engines — deve sempre vir acompanhado do Physics Step Time.

---

## 2. Setup do Cenário

### 2.1 Variáveis de Controle

| Parâmetro | Valor recomendado | Por quê |
|---|---|---|
| **Fixed Timestep** | 0.02s (50 Hz) | Igual ao Cenário 1; padrão do assignment |
| **Forma dos objetos** | Esfera (SphereCollider) | A forma mais simples para narrowphase; evita instabilidades de box-box |
| **Raio das esferas** | 0.5m | Tamanho suficiente para visibilidade; pequeno o suficiente para caber 10k no funil |
| **Massa** | 1 kg por objeto | Uniforme |
| **Restituição** | 0.3 | Um pouco de bounce é ok — estamos testando dinâmica, não estabilidade |
| **Atrito** | 0.4 | Padrão |
| **Gravidade** | -9.81 m/s² | Padrão |
| **Renderização** | Desativada ou mesh mínima | Objetivo é medir física, não render; usar primitivas simples |

### 2.2 Geometria do Funil

O funil serve para **forçar interação entre todos os corpos** — sem ele, objetos se dispersariam e o número de colisões simultâneas seria baixo. O funil cria uma zona de alta densidade de contatos.

```
  /    \            ← boca larga: objetos entram aqui
 /      \
/        \          ← paredes inclinadas (Static Bodies)
|        |          ← gargalo estreito (opcional)
```

Especificações do funil:
- Construído com **Static Bodies** (não simulados, sem custo de solver)
- Paredes inclinadas em ~45–60°
- Gargalo deve ser estreito o suficiente para acumular os objetos, mas não bloquear completamente
- Profundidade suficiente para acomodar os 10k objetos sem overflow

> **Alternativa ao funil:** uma caixa fechada (6 paredes estáticas). Mais simples de implementar, resultado similar. Use se o funil for difícil de construir de forma idêntica nas 3 engines.

### 2.3 Spawn dos Objetos

Os objetos devem ser instanciados **todos de uma vez** (ou em uma única chamada de `FixedUpdate`) no início do teste — não gradualmente. Isso garante que o pico de estresse seja imediato e mensurável.

**Posição de spawn:** acima da boca do funil, com posições ligeiramente randomizadas para evitar sobreposição inicial (que causaria explosão de corpos e invalidaria o teste).

**Pseudocódigo de spawn:**
```
func iniciar_teste():
    timestamp_inicio = tempo_atual()
    
    for i in range(NUM_OBJETOS):
        obj = instanciar_esfera_rigida()
        obj.posicao = posicao_aleatoria_dentro_da_area_de_spawn()
        obj.velocidade_inicial = Vector3(0, 0, 0)
        adicionar_a_cena(obj)
    
    iniciar_coleta_de_metricas()
```

---

## 3. O Que Medir e Como

### 3.1 Métrica Principal: Physics Step Time

**Definição:** o tempo (em ms ou µs) que o motor gastou executando **um único passo de física** (fixed timestep).

Esta métrica deve ser medida via o **Profiler interno de cada engine** — não pode ser estimada via `delta_time` do game loop, pois inclui overhead de rendering e outros sistemas.

**O que acessar em cada engine (conceito genérico):**
- Unity: `Profiler API` — contador `"Physics.Processing"` ou `"Physics.Simulate"`
- Unreal: `stat physics` no console em runtime — linha `"PhysicsTime"` ou `"Physics"`
- Godot: `Performance.get_monitor(Performance.PHYSICS_PROCESS_TIME)` — retorna tempo em microsegundos

**Quando medir:** coletar amostras **durante a fase de estresse máximo** — os primeiros 5–10 segundos após o spawn, antes de os objetos se acomodarem. A média deve ser calculada sobre esse período, não sobre toda a duração do teste.

**Como automatizar:**
```
metricas = []
janela_de_coleta = 10  # segundos

func a_cada_passo_de_fisica():
    if tempo_decorrido > janela_de_coleta:
        finalizar_coleta()
        return
    
    step_time = ler_physics_step_time_do_profiler()
    metricas.append(step_time)

func finalizar_coleta():
    media = calcular_media(metricas)
    desvio = calcular_desvio_padrao(metricas)
    exportar_csv(media, desvio, metricas)
```

### 3.2 Métrica Secundária: FPS

**Como medir:** a maioria das engines expõe FPS via API de performance (ex: `Engine.get_frames_per_second()` no Godot, `1.0 / Time.deltaTime` no Unity, `GAverageFPS` no Unreal).

**Medir em simultâneo com Physics Step Time**, na mesma janela de tempo. Registrar a média de FPS durante o período de estresse máximo.

### 3.3 Ponto de Quebra (Break Point)

Além das médias, identificar **em qual variação** o Physics Step Time supera 20ms (o Fixed Timestep). Isso define o limite prático de cada engine:

| Variação | Physics Step Time médio | > 20ms? |
|---|---|---|
| 1.000 objetos | X ms | Sim/Não |
| 5.000 objetos | Y ms | Sim/Não |
| 10.000 objetos | Z ms | Sim/Não |

---

## 4. Metodologia de Coleta (10 Execuções × 3 Variações)

### 4.1 Estrutura das Execuções

Para cada variação (1k, 5k, 10k), em cada engine:

1. **Resetar o estado da simulação completamente** (não apenas destruir os objetos — reiniciar a cena inteira para zerar buffers de broadphase e islands do solver)
2. Spawn de todos os objetos
3. Coletar Physics Step Time médio nos primeiros N segundos (definir N igual nas 3 engines — sugestão: 10s)
4. Registrar o valor
5. Repetir 10 vezes

**Total de medições:** 3 engines × 3 variações × 10 runs = **90 medições** de Physics Step Time.

### 4.2 Tabela de Coleta

```
Engine: _____ | Variação: _____ objetos | Janela de coleta: _____s
| # | Physics Step Time médio (ms) | FPS médio | Dentro da faixa [M±σ]? |
|---|---|---|---|
| 1 | | | |
| 2 | | | |
| ... | | | |
| 10 | | | |
| Média bruta (M) | | | |
| Desvio Padrão (σ) | | | |
| Faixa [M-σ, M+σ] | | | |
| Execuções descartadas | | | |
| **Média Final** | | | |
```

### 4.3 Automatização da Coleta

A automatização é crítica para a variação de 10k objetos — rodar 10 vezes manualmente é inviável e introduz erro humano. Cada engine deve ter um script que:

1. Inicia a cena
2. Spawna os objetos
3. Coleta métricas durante a janela
4. Ao final da janela, escreve os dados em um arquivo CSV
5. Reinicia a cena (via código, sem interação manual)
6. Repete até completar 10 execuções
7. Encerra

**Formato de saída CSV sugerido:**
```
engine,scenario,variation,run,physics_step_time_ms,fps
unity,chuva,1000,1,4.2,144
unity,chuva,1000,2,4.5,138
...
```

Isso permite carregar todos os dados no mesmo script Python/Excel para calcular estatísticas e gerar gráficos.

---

## 5. Armadilhas Comuns

| Problema | Causa | Solução |
|---|---|---|
| Physics Step Time inclui rendering | Medindo wall time do frame inteiro | Usar Profiler API específico de física, não `Time.deltaTime` |
| Spike nas primeiras 2–3 execuções | Cache frio, JIT compilation (Unity/Godot C#) | Descartar as 2 primeiras execuções como warm-up (ou incluí-las e deixar o filtro ±σ eliminar) |
| Objetos se sobrepõem no spawn e "explodem" | Spawn sem aleatorização | Usar grid ou posições aleatórias dentro da área de spawn |
| FPS muito alto mascara custo de física | V-Sync ativo | Desativar V-Sync antes dos testes |
| Resultados inconsistentes entre sessões | Background processes | Fechar outros programas; testar em horário estável |
| Step Time = 0 em alguns frames | Engine dormindo islands completas | Normal para 1k com poucos contatos; verificar se os objetos realmente estão em movimento durante a janela |
| Funil diferente entre engines | Construção manual | Usar as mesmas dimensões — documenta-las explicitamente |

---

## 6. Interpretação dos Resultados

### 6.1 Curva de Escalabilidade

O valor mais revelador não é o número absoluto de ms, mas a **taxa de crescimento** do Physics Step Time em função do número de objetos.

Uma engine com boa paralelização deve crescer sub-linearmente:
- 1k → 5k (5× mais objetos) → Step Time cresce 3× → **boa escalabilidade**
- 1k → 5k (5× mais objetos) → Step Time cresce 8× → **escalabilidade ruim** (broadphase ou solver não paralelizado)

### 6.2 O Que Revelará Sobre Cada Engine

- **Unity (PhysX clássico):** broadphase SAP/MBP com suporte a multi-threading parcial; solver Gauss-Seidel com ~4 iterações default. Esperado: bom para 1k, degradação para 5k+.
- **Unity (DOTS Physics):** Job System paraleliza broadphase e solver; Incremental BVH desde 1.3. Esperado: melhor escalabilidade que PhysX clássico.
- **Unreal (Chaos):** islands resolvidos em paralelo, Async Physics Tick; mas custo de LWC (double precision) eleva o custo base por objeto. Esperado: FPS estável por conta do async, mas Physics Step Time absoluto pode ser alto.
- **Godot (Jolt):** multi-threaded nativo com job system próprio. Esperado: boa escalabilidade comparada ao GodotPhysics; mas sem benchmark público para 10k esferas.

---

## 7. Próximos Passos

Com este documento como base, os próximos arquivos a criar são as implementações específicas por engine:

- `scenario-2-chuva-unity.md` — C# + Profiler API + automação de 10 runs
- `scenario-2-chuva-unreal.md` — Blueprint/C++ + `stat physics` + automação
- `scenario-2-chuva-godot.md` — GDScript + `Performance` API + automação
