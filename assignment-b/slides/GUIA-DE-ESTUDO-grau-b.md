# Guia de Estudo — Grau B (Benchmark de Physics Engines)

> Companion do deck `grau-b.html`. Objetivo: você **entender e defender** cada conceito, número
> e decisão de método na frente da professora. Termos técnicos ficam em inglês (é o padrão da
> área); a explicação é em português. Números são do dataset **canônico n=15** (`results/RESULTS.md`).

## Índice
1. [A ideia geral (o que estamos medindo)](#1-a-ideia-geral)
2. [Glossário — os termos que você PRECISA saber explicar](#2-glossário)
3. [As 3 engines e suas arquiteturas](#3-as-3-engines)
4. [Cenário 1 — A Torre (estabilidade)](#4-cenário-1--a-torre)
5. [Cenário 2 — A Chuva (desempenho)](#5-cenário-2--a-chuva)
6. [O método estatístico (M±σ)](#6-o-método-estatístico)
7. [Análise crítica — previsão × realidade](#7-análise-crítica)
8. [Bastidores — erros, experimentos, honestidade](#8-bastidores)
9. [Colinha de números (decorar)](#9-colinha-de-números)
10. [Perguntas prováveis do professor + respostas](#10-perguntas-prováveis)

---

## 1. A ideia geral

**Grau A** foi teórico: levantamos como cada motor de física é construído por dentro (solvers,
threading, CCD). **Grau B** é a prova prática: rodamos **a mesma cena** nas três engines e medimos
os limites reais, com rigor estatístico.

Dois cenários, cada um estressa uma coisa diferente:

| Cenário | O que estressa | Métrica primária |
|---|---|---|
| **A Torre** | a **estabilidade** do solver (uma pilha alta de cubos) | **colapso** (`kept%`) |
| **A Chuva** | o **desempenho** sob milhares de colisões simultâneas | **Physics Step Time** (ms) |

A pergunta que amarra tudo: **os resultados confirmam o que a arquitetura do Grau A previu?**

**Regra de ouro do experimento:** só o *motor de física* pode mudar entre as engines. Tudo o mais
é travado igual: `fixed timestep 0,02 s (50 Hz)`, gravidade ≈ −9,81, massa 1 kg, atrito 0,5, formas
primitivas, cena 100% gerada em código. Isso é o que torna a comparação **justa**.

---

## 2. Glossário

Se a professora perguntar "o que é X?", você tem que responder isto sem gaguejar.

**Solver** — a parte do motor que resolve as *constraints* (contatos, junções) a cada passo:
calcula as forças/impulsos que impedem os corpos de se atravessarem. É o coração da simulação.

**Iterações do solver** — quantas vezes ele repassa as constraints por passo. Mais iterações =
mais preciso, mais caro. Unity default 6, Chaos 8. **Achado importante nosso:** subir iterações
(6→20 no Unity) **não** resolve o colapso da torre — é o *algoritmo* que importa, não a contagem.

**PGS (Projected Gauss-Seidel)** — solver iterativo do PhysX (Unity, default). Resolve uma
constraint por vez, em varredura. Converge **devagar** em pilhas altas: com poucas iterações,
sub-resolve a força na base → a coluna afunda.

**TGS (Temporal Gauss-Seidel)** — variante *opt-in* do PhysX que faz *substeps* e reavalia a
posição no meio do passo → converge melhor em pilhas. No nosso teste, o TGS **sobe 1 degrau**
(segura N=15 onde o PGS colapsa), mas ainda colapsa em N=20.

**XPBD (Extended Position-Based Dynamics)** — solver do Chaos (Unreal). Trabalha por *posição*
(não por força), com constraints "compliantes" (com rigidez ajustável). A ideia (Macklin 2019,
"Small Steps"): passos pequenos deixam a simulação mais estável.

**Shock Propagation** — mecanismo do Chaos, ligado por default, feito **exatamente** para pilhas:
processa os contatos em camadas de baixo para cima, tratando os corpos de baixo como "mais pesados"
/ quase imóveis. Por isso o Chaos foi o único a segurar ~⅓ da pilha em N=15.

**LWC (Large World Coordinates)** — a Unreal usa coordenadas de mundo de alta precisão (mundos
gigantes). Isso **eleva o custo base** de cada operação de física — parte de por que o Chaos é o
mais caro.

**Broadphase** — 1ª etapa da colisão: acha *pares candidatos* a colidir de forma barata (sem testar
geometria exata). **SAP (Sweep and Prune)** é o algoritmo do PhysX. Com milhares de corpos, é aqui
que o número de pares pode explodir.

**Narrowphase** — 2ª etapa: para cada par candidato, testa a colisão **exata** (geometria contra
geometria) e gera os pontos de contato.

**CCD (Continuous Collision Detection)** — detecção que evita "tunneling" (corpo rápido atravessar
outro entre dois passos). Custa caro; parte do bundle que tentamos cortar no Chaos.

**Sleep** — quando um corpo fica praticamente parado, o motor o coloca para "dormir" (para de
simulá-lo) para economizar CPU. **Cada engine acorda/dorme a um limiar diferente** (Unity 0,005 m/s
· Jolt 0,03 m/s · Chaos 5 frames) → por isso "tempo até dormir" **não** compara entre engines.

**Jitter** — tremor residual: o corpo deveria estar parado mas vibra porque o solver não convergiu.
Sintoma de pilha mal resolvida.

**Warm starting** — o solver reaproveita os impulsos do passo anterior como chute inicial → converge
mais rápido em cenas quase estáticas.

**Timestep fixo (0,02 s = 50 Hz)** — a física avança em passos fixos de 20 ms, independente do FPS.
Determinístico e reprodutível. Exigência do spec, igual nas três.

**Physics Step Time** — o tempo de **CPU** para avançar **um** passo de física (esses 0,02 s) com N
corpos. **É a métrica que compara** desempenho — mede a mesma grandeza física nas três.

**FPS** — ritmo do laço de **render** (desenho na tela). **NÃO compara** entre engines (explico no §5).

**Acoplamento render↔física (coupling)** — na Unreal a física roda **acoplada** ao frame (1 passo
por frame, *Fixed Frame Rate* 50 Hz), então FPS ≈ 1000/step (teto 50). Em Unity/Godot render e
física são **desacoplados** → o "FPS" é velocidade de GPU, não custo de física.

**Pancaking** — modo de falha da torre: a coluna **achata reta para baixo** (o topo cai de forma
monotônica). Diferente de *explosão* (corpos voando com velocidade altíssima). Os nossos colapsos
são pancaking (velocidade máx ~40 m/s, compatível com queda por gravidade).

**Espiral da morte (spiral of death)** — se um passo de física demora **mais** que o timestep, o
motor tenta compensar rodando mais passos, e afunda cada vez mais. É o pior cenário de desempenho.

**Job system / islands** — formas de **multi-threading**. Jolt tem um *job system* nativo; Chaos
agrupa corpos em *solver islands*. A ideia é escalar em CPUs multi-core (a nossa é 10 núcleos).

**`kept%` (colapso)** — altura final do topo ÷ altura se a pilha estivesse intacta. Puramente
**geométrica** → independe do limiar de sleep → **comparável entre engines**. Veredito:
**ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%**.

---

## 3. As 3 engines

| | **Unity** | **Godot** | **Unreal** |
|---|---|---|---|
| Versão | 6000.5 (U6 LTS) | 4.7 | 5.8 |
| Motor de física | **PhysX** | **Jolt** | **Chaos** |
| Solver (default) | **PGS** (6+1 iter) | impulse (Gauss-Seidel) | **XPBD + Shock Propagation** (8+2 iter) |
| Sleep threshold | 0,005 m/s | 0,03 m/s | 5 frames |
| Física ↔ render | desacoplada | desacoplada | **acoplada** (trava frame a 50 Hz) |
| Step time medido via | `Stopwatch(Physics.Simulate)` | `TIME_PHYSICS_PROCESS` | `TG_Pre→PostPhysics` |

**Ponto-chave para a professora:** deixar solver e sleep no **padrão de fábrica** de cada engine é
**exigência do spec (§2.1: "não modificar — é parte do que se compara")**, não descuido. Cada engine
mede o step time no seu **mecanismo nativo**, mas todos medindo a mesma coisa: "custo de avançar 1
passo de 0,02 s".

---

## 4. Cenário 1 — A Torre

**O teste:** empilhar N cubos rígidos e ver se a pilha se sustenta. Uma pilha alta é **brutal** para
o solver — o contato da base tem que segurar o peso de dezenas de cubos. Com poucas iterações, o
solver sub-resolve essa força a cada passo → a coluna afunda (*pancaking*).

**Por que lemos por `kept%` (colapso) e NÃO por tempo-até-sleep?**
Porque cada engine dorme a um limiar diferente (0,005 · 0,03 · 5 frames). O "tempo até dormir" mede
coisas diferentes em cada uma → não é comparável 1-a-1. Já o `kept%` é **geometria pura** (quanto da
pilha sobrou de pé), igual nas três. Exemplo gritante do porquê: Godot sweep N=10 tem `kept=100%`
(estável) mas tempos variando de 0,5 s a 11 s na mesma config — o **tempo é ruidoso, o colapso é limpo**.

### Resultados da Torre

**N = 100 cubos:** as **três colapsam igual** — `kept ~2%` (pancaking). Nenhuma sustenta 100 cubos
rígidos com os defaults. Foi medido 15/15 em cada engine.

**Varredura de N crescente (o "degrau" do Chaos):**

| N | Unity · PGS | Godot · Jolt | Unreal · Chaos |
|---|---|---|---|
| 10 | ✓ estável | ✓ estável | ✓ estável |
| 15 | 💥 colapsa (6%) | 💥 colapsa (22%) | ⚠️ **parcial (38%)** |
| 20 | 💥 colapsa (9%) | 💥 colapsa (10%) | 💥 colapsa (8%) |

**A leitura:** só o Chaos tem **Shock Propagation** (default) — segura ~⅓ da pilha em N=15 onde os
outros achatam. **Mas o efeito é limitado:** em N=20 todos colapsam. **Conclusão honesta:** "pilha
rígida alta estável" **não existe de fábrica em nenhuma** engine — por isso o design de jogos evita
pilhas rígidas altas.

> **Ressalva honesta (diga se perguntarem):** não desligamos o Shock Propagation para *provar* a
> causa; e o `max_v ~19,5 m/s` em N=15 mostra que é colapso **parcial** (a metade de cima cede), não
> "segurar" a pilha inteira. E a varredura PGS×TGS do Unity foi n=3 (exploratória) — não apoiamos
> tese forte só nela.

---

## 5. Cenário 2 — A Chuva

**O teste:** 1.000 / 5.000 / 10.000 esferas caindo juntas numa caixa fechada. Isso **satura o
pipeline**: a broadphase explode em número de pares, a narrowphase testa cada contato, o solver
resolve tudo por passo.

### Physics Step Time (a métrica que compara) — Média Final n=15

| Esferas | Unity · PhysX | Godot · Jolt | Unreal · Chaos |
|---|---|---|---|
| 1.000 | **1,5 ms** | 3,0 ms | 10,4 ms |
| 5.000 | **7,1 ms** | 15,2 ms | 67,1 ms |
| 10.000 | **13,3 ms** | 32,8 ms | 146,6 ms |

**Classificação: Unity/PhysX < Godot/Jolt < Unreal/Chaos.** O PhysX é o mais barato em toda escala;
o Chaos custa **≈11× o PhysX** em 10k. O Chaos paga XPBD + LWC + acoplamento + editor.

**Custo relativo (× vs PhysX, o mais barato), em 10k:** PhysX **1×** · Jolt **≈2,5×** · Chaos **≈11×**.
Essa é a "história da razão" que os ms absolutos escondem — por isso o gráfico do deck usa **escala
log** no eixo Y (cada divisória = 10×), o que deixa a barrinha do PhysX legível ao lado do Chaos.

### FPS — a ressalva mais importante do trabalho

| Esferas | Unity | Godot | Unreal |
|---|---|---|---|
| 1.000 | 233 | **1695** | 50 |
| 5.000 | 121 | 431 | 14 |
| 10.000 | 46 | 15 | 6,7 |

**Por que o FPS NÃO compara entre engines?** Por causa do **acoplamento**:
- **Unreal (acoplada):** 1 passo de física por frame → `FPS ≈ 1000/step_ms`, com teto de 50 (Fixed
  Frame Rate). Aqui o FPS *reflete* o custo da física.
- **Unity/Godot (desacoplada):** o render roda **livre** → o "FPS" é velocidade de GPU, **não** custo
  de física. Ex.: Godot faz 1695 FPS em 1k rodando a física a apenas 50 Hz.

Por isso o **Step Time compara** (mesma grandeza física) e o **FPS só vale por engine** (tendência).
O `amostras=500` em todas confirma que as três mediram o **mesmo trecho simulado** (10 s de física).

### Como cada motor escala

| | 1k→10k (auto-escala) | 5k→10k | vs PhysX (10k) |
|---|---|---|---|
| Unity/PhysX | **8,7×** | 1,9× | 1× (baseline) |
| Godot/Jolt | **11,0×** | 2,2× | ≈2,5× |
| Unreal/Chaos | **14,1×** | 2,2× | ≈11× |

> **Ressalva (importante!):** o `5k→10k` é **sublinear nas três** (dobra a carga, custo sobe só ~2×).
> Isso é **efeito da janela de medição** (ela captura a pilha já assentada, não só a queda), **não**
> uma vantagem específica do Jolt. Não caia na tentação de dizer "o Jolt escala melhor" — não é isso.

---

## 6. O método estatístico

Uma única execução não é confiável (ruído do SO, agendamento de threads, boost/throttling da CPU).
Por config, por engine:

1. Rodar **15 execuções isoladas** (acima do mínimo de 10 do enunciado).
2. Calcular **M** (média) e **σ** (desvio **populacional**, ÷N — como o `howto-statistics.md` implica).
3. **Descartar** toda execução fora de `[M − σ, M + σ]`.
4. **Média Final** = média só das que sobraram.

**Ponto que a professora pode cutucar:** "vocês descartaram muita coisa!" — Resposta: **descartar
~⅓ é ESPERADO, não sinal de dado ruim.** Numa distribuição normal, a faixa ±1σ contém ~68% dos
valores; logo ~32% caem fora **por definição**. E quando a engine é ultra-consistente (ex.: Unity 1k
FPS varia só 230–235, σ=1,4), a faixa fica estreitíssima e o filtro corta 7/15 — mas como os valores
são quase idênticos, a Média Final é praticamente igual à média bruta. **Contagem de descarte alta ≠
instabilidade.**

**Duas exceções em que a Média Final perde sentido** (e aí usamos **mediana + grupos**):
- **Distribuição bimodal:** os valores formam dois grupos. Caso real: **Godot Chuva 10k FPS** =
  `98, 26, 14, 10, 9, ...` (um cluster ~10, alguns 40–98). A média não representa nenhum grupo (a
  faixa ±σ chega a incluir valor negativo).
- **Timeout:** uma run que bate 60 s sem a pilha dormir é **resultado físico válido** (a pilha nunca
  assenta), **não** um outlier a remover. Entra no cálculo.

---

## 7. Análise crítica

Guardamos 3 previsões do Grau A e confrontamos com o dado:

| Previsão do Grau A | Veredito | Detalhe |
|---|---|---|
| Shock Propagation ajuda a Torre? | ✅ **Confirmou** | Chaos é o único a segurar 1 degrau a mais (N=15, 38% parcial) |
| Chaos é o mais caro? | ✅ **Confirmou** | 146,6 ms em 10k, ≈11× o PhysX (LWC + XPBD) |
| Jolt escala melhor (é o mais rápido)? | ❌ **Inverteu** | Quem foi mais barato em **toda** escala foi o **PhysX** (Unity); o Jolt ficou no meio (≈2,5× o PhysX) |

**Além disso, 2 correções de método que a prática nos forçou a fazer:**
1. O solver **default** do Unity é **PGS**, não TGS (a gente tinha assumido TGS no Grau A). Corrigido.
2. A "diferença de 11×" é **custo bruto da nossa configuração** (editor + física acoplada +
   sincronização de 10k atores no Unreal), **não** só do solver. Não é honesto dizer "o solver do
   Chaos é 11× mais lento" — a medição do Unreal engloba mais coisa.

**Frase de fechamento:** *"A arquitetura do Grau A previu bem o comportamento — e o benchmark o
tornou mensurável, corrigindo pelo caminho duas suposições (PGS≠TGS; a leitura da diferença do Chaos)."*

---

## 8. Bastidores

### Erros que cometemos e corrigimos (isto IMPRESSIONA — mostra rigor)

| Problema | Correção | Impacto |
|---|---|---|
| Janela em **wall-clock**: em 10k, 10 s reais = só ~2 s simulados (quase só queda livre) | Janela por **tempo de simulação** (500 passos), igual nas 3 | Chaos 10k saltou **~86 → 147 ms** (passou a medir a pilha densa de verdade) |
| Unity: `ProfilerRecorder` retornava **0** no step time | `Stopwatch` em volta de `Physics.Simulate()` | métrica nunca mais veio 0 |
| Assumimos **TGS** no Unity (herança do Grau A) | Config real = **PGS** (`m_SolverType:0`) | Grau A corrigido |
| Jolt: buffer de contatos estourou em 10k (travava na 2ª execução) | Tunar buffers do Jolt + **reset in-place** entre execuções | 10k passou a rodar sem travar |

### Experimentos deliberados

- **"Otimizar" o Chaos** (cortar iterações 8→4 + CVars): resultado = **+19% MAIS LENTO** em 10k
  (146,6 → 173,8 ms). **Aprendizado:** o gargalo é o pipeline de **colisão**, não as iterações. Uma
  otimização que falha, bem documentada, ensina tanto quanto uma que funciona.
- **PGS vs TGS vs +iterações** (Unity): iterações 6→20 **não movem** o colapso; TGS (opt-in) sobe
  **1 degrau**. É o **algoritmo**, não a contagem.
- **Arena fechada** (decisão de método): paredes + teto para conter cubos ejetados → métrica de
  colapso válida nas três, sem timeouts falsos.

### Reprodutibilidade (n=10 × n=15)

Rodamos **dois lotes independentes**: um piloto a 10 execuções e o final a 15. Se o número mal se
move ao subir de 10→15, ele é confiável.

| Step Time 10k (ms) | Unity | Godot | Unreal |
|---|---|---|---|
| n=10 (piloto) | 13,1 | 32,9 | 149,8 |
| n=15 (canônico) | 13,3 | 32,8 | 146,6 |
| Δ | +1,7% | −0,1% | −2,2% |

Toda a Chuva se moveu **< 7%** (pior caso Unity 1k = +6,4%; o 10k acima ficou ≤ 2,2%), e **nenhuma
conclusão mudou** (classificação, ≈11×, vereditos da Torre). **n=15 é o canônico**; o n=10 fica
arquivado só para esta comparação.

---

## 9. Colinha de números

**Step Time (ms) — 1k / 5k / 10k:**
- Unity: 1,5 / 7,1 / 13,3
- Godot: 3,0 / 15,2 / 32,8
- Unreal: 10,4 / 67,1 / 146,6

**Razões:** Chaos ≈ **11×** PhysX (10k) · Jolt ≈ **2,5×** PhysX (10k) · sublinear 5k→10k ≈ **2×** nas três.

**FPS (1k/5k/10k):** Unity 233/121/46 · Godot 1695/431/15 · Unreal 50/14/6,7.

**Torre kept%:** N=10 todas estáveis · N=15 Unity **6%** / Godot **22%** / Unreal **38% (parcial)** ·
N=20 todas colapsam · N=100 todas ~**2%**.

**Vereditos:** ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%.

**Método:** 15 runs → M, σ (÷N) → descarta fora de [M−σ, M+σ] → Média Final. ~⅓ de descarte é normal.

**Total:** ≈315 execuções cronometradas (Chuva 135 + Torre varredura 135 + N=100 45 + exploratórios).

---

## 10. Perguntas prováveis

**"Por que o Unreal ficou tão mais lento? O Chaos é ruim?"**
Não necessariamente. A nossa medição do Unreal é **custo bruto da configuração**: editor + física
**acoplada** ao frame + sincronização de 10k atores + **LWC** (coordenadas de alta precisão) + XPBD.
Não estamos isolando o solver puro. Num build *Shipping* + *Async Physics Tick*, boa parte desse
custo some. A comparação é justa como "custo de rodar 10k corpos naquele editor", não como "o solver
do Chaos é 11× pior".

**"Por que não compararam o FPS direto? Seria mais intuitivo."**
Porque o FPS mede coisas diferentes em cada engine. Na Unreal ele é capado em 50 e reflete a física;
em Unity/Godot é velocidade de GPU (render desacoplado) — Godot faz 1695 FPS rodando a física a 50 Hz.
Comparar FPS seria comparar laranja com maçã. O **Physics Step Time** mede a mesma grandeza física
nas três, então é ele que compara.

**"Vocês descartaram 5, 7 execuções... não é manipular o dado?"**
Não — é o filtro **M±σ do enunciado**, aplicado igual em todas. Descartar ~32% é esperado (±1σ contém
~68% numa normal). E quando a engine é super consistente, o filtro corta muito mas a Média Final quase
não muda, porque os valores são quase idênticos. Contagem de descarte alta ≠ dado ruim.

**"Por que não mexeram nas iterações / no solver para segurar a torre?"**
Duas razões. (1) O spec **proíbe** — o comportamento default é parte do que se compara. (2) Testamos
mesmo assim como experimento: subir iterações 6→20 **não** move o colapso; trocar PGS→TGS sobe só 1
degrau. É o **algoritmo** que importa, não a contagem — e nenhum default segura pilha alta.

**"O Chaos 'segurou' a torre em N=15 mesmo?"**
Parcialmente. `kept=38%` é colapso **PARCIAL** — a metade de cima cede, mas ~⅓ fica de pé, graças ao
**Shock Propagation** (que processa contatos de baixo para cima). Os outros dois já achatam em N=15.
Mas em N=20 todos colapsam — o efeito é real e limitado.

**"Como sabem que o resultado é confiável?"**
Reprodutibilidade: dois lotes independentes (n=10 e n=15) dão a mesma classificação, e toda a Chuva
se moveu < 7% ao subir de 10→15 execuções. Nenhuma conclusão mudou. Além disso, cada número só valeu
depois de pegarmos e corrigirmos erros de método (a janela wall-clock, o step time zerado do Unity).

**"O que é 'sublinear' e por que aparece?"**
De 5k para 10k a carga dobra, mas o custo sobe só ~2× (na verdade menos, ~1,9–2,2×). Isso é da
**janela de medição**: ela mede a pilha já assentada (contatos estáveis, corpos dormindo), não só o
caos da queda. É efeito de medição, não vantagem de engine — por isso não atribuímos ao Jolt.

**"Qual engine é 'a melhor' então?"**
Depende do critério. Em **custo de física** (Step Time), PhysX/Unity ganha folgado. Em **estabilidade
de pilha**, Chaos aguenta 1 degrau a mais, mas nenhuma sustenta pilha alta com defaults. A lição de
projeto: para jogos, evita-se pilha rígida alta justamente porque nenhum solver a resolve de graça.

---

*Fontes: `results/RESULTS.md` (n=15), `results/COMPARISON-n10-vs-n15.md`,
`results/METRICS-e-exclusoes.md`. Deck: `slides/grau-b.html`.*
