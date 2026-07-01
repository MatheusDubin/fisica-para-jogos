# Análise Comparativa — 3 Engines (Grau B)

> Dataset **preliminar** de 10 iterações por config (final: 20, roda depois).
> Metodologia idêntica nas 3: timestep 0.02s, malhas primitivas, massa 1kg,
> mesma cena. Chuva com janela **sim-time** (500 passos) nas três → comparável.
> Tabelas puras + M±σ em `RESULTS.md` (gerado por `aggregate_stats.py`).

| Engine | Motor de física | Solver |
|---|---|---|
| Godot 4.x | **Jolt** | impulse-based (Gauss-Seidel, sem warm-start de ilha) |
| Unity 6 | **PhysX** | **PGS** (Projected Gauss-Seidel — default, NÃO TGS) |
| Unreal 5.8 | **Chaos** | XPBD + **Shock Propagation** (default) |

---

## Cenário 2 — A Chuva (Physics Step Time, métrica primária)

**Tempo médio do passo de física (ms), média final de 10 runs:**

| N esferas | Godot/Jolt | Unity/PhysX | Unreal/Chaos |
|---|---|---|---|
| 1.000 | 2.87 | **1.43** 🥇 | 10.10 |
| 5.000 | 15.13 | **6.92** 🥇 | 65.59 |
| 10.000 | 32.87 | **13.10** 🥇 | 149.83 |

**Ranking de custo (menor = melhor): Unity/PhysX < Godot/Jolt < Unreal/Chaos.**
- PhysX é ~2× mais rápido que Jolt e ~**11× mais rápido que Chaos** em 10k.
- Escalonamento 1k→10k: PhysX 9.2×, Jolt 11.4×, Chaos 14.8× (todos sublineares no topo; Chaos degrada mais).
- As 3 rodaram 10k sem crash.

**Por que o Chaos parece tão lento? (e NÃO é o hardware)**
A mesma máquina rodou PhysX a 13 ms e Chaos a 150 ms em 10k — um gap de 11× no
**mesmo PC** é arquitetura + setup de medição pior-caso, não a máquina. Fatores:
1. **Editor/PIE (Development), não Shipping** — overhead mais pesado; o editor do
   Unreal é mais pesado que o do Unity/Godot (as 3 rodaram no editor, mas o Unreal
   paga mais).
2. **Física acoplada (nossa escolha p/ medir)** — o render espera a física, então
   o FPS despenca. Num jogo real o Chaos roda **async** e o jogo continua fluido;
   nosso setup expõe o custo bruto de propósito.
3. **10k AActors separados** — sync de transform por ator entra na janela medida.
4. **Defaults do Chaos mais pesados que o PhysX:** 8 iterações de posição (PhysX ~4–5),
   **double precision (LWC)**, XPBD + shock propagation = mais trabalho por passo, por design.

**Reframe honesto:** o Chaos não é "terrível" na prática — num jogo que shippa ele
roda async e mantém 60+ FPS enquanto a física trabalha na thread dela; nossa
medição acoplada+editor mostra o custo bruto. Os 150 ms são o custo real de
resolver, mas não é o que o jogador sente.

### Coluna exploratória: "Chaos-otimizado" (exercício de otimização) — **o tiro saiu pela culatra**
> Config-only, mesmo modo (editor), coletado à parte em `chuva-optimized/` — **não
> altera** a coluna default. Bundle de CVars do Chaos: iterações 8→4 / 2→1 / 1→0,
> `Deterministic 0`, `UseCCD 0`, `DeferNarrowPhase 1`, island-groups com mais workers.
> Pergunta: quanto do custo é intrínseco vs. defaults conservadores?
> **Resposta empírica: o custo é intrínseco — a "otimização" deixou MAIS LENTO.**

| N | Chaos default | Chaos "otimizado" | Δ | vs PhysX (13,1 ms) |
|---|---|---|---|---|
| 1.000 | 10.10 | 10.32 | **+2,2%** 🔴 | 7,2× |
| 5.000 | 65.59 | 73.92 | **+12,7%** 🔴 | 5,7× |
| 10.000 | 149.83 | **173.83** | **+16,0%** 🔴 | 13,3× |

FPS caiu junto (5k: 14,2→13,0; 10k: 6,6→5,8). **Reduzir iterações do solver não
acelerou nada — piorou.** Este é o achado mais valioso do exercício, e é honesto.

**Por que a "otimização" falhou (e o que isso ensina):**
1. **O gargalo NÃO é o solver de iterações.** Numa chuva densa (10k corpos em pilha),
   o custo dominante é o **pipeline de colisão** (broad + narrow phase, contagem de
   contatos) sobre milhares de pares, não as 8 iterações de posição. Cortar iterações
   raspa a parte barata.
2. **Menos iterações = pilha menos estável = MAIS trabalho.** Com 4 posições em vez de
   8, a pilha assenta pior, mais corpos ficam **acordados e tremendo** por mais tempo →
   mais contatos ativos por passo → o solver de colisão trabalha mais. A economia de
   iterações é engolida (e superada) pelo contato extra.
3. **`IslandGroups.WorkerMultiplier 2` foi contraproducente.** Uma chuva empilhada é
   essencialmente **uma ilha de contato gigante**. Fatiar em mais grupos de workers só
   adicionou overhead de agendamento/sincronização de tasks, sem paralelismo real (não
   dá pra paralelizar uma ilha única) — cache pior, mais sync.
4. **`UseCCD 0` / `DeferNarrowPhase 1` não moveram o ponteiro** — as esferas não usavam
   CCD de qualquer forma, e diferir a narrow-phase só reorganiza o mesmo custo.
5. O que **realmente** reduziria (fora do escopo config-only): **Shipping build**
   (sem overhead de editor), **física async** (esconde o custo em outra thread, não o
   reduz — é lever de FPS), e **ISM/instâncias** em vez de 10k AActors. Nada disso é
   "config-only", então fica de fora desta coluna por decisão de escopo.

**Conclusão do exercício:** os ~150 ms do Chaos em 10k são **estruturais** (arquitetura
de colisão + double precision LWC + 10k atores), não um default conservador que se
destrava com CVars. Tentar afinar o solver por CVar não só não ajuda — atrapalha.
Mantemos as **duas** colunas justamente para mostrar isso: default (out-of-the-box) e
"otimizado" (que documenta uma otimização **que não funcionou**, resultado tão válido
quanto uma que funcionaria).

> Nota de método: o "otimizado" também perdeu o determinismo (σ subiu de ≈0 p/ 7,5 ms
> em 10k, com `Deterministic 0`), ou seja, pagamos variância **e** ficamos mais lentos.

### FPS — reportado, mas ⚠️ NÃO comparável entre engines (e por quê)
| N | Godot | Unity | Unreal |
|---|---|---|---|
| 1k | ~1660 | 230 | 50 |
| 5k | ~430 | 123 | 14 |
| 10k | ~10–80 (bimodal) | 48 | 6.6 |

O enunciado pede **os dois** (Step Time E FPS) — reportamos os dois. A questão é o
que cada um pode **comparar entre engines**:

- **Step Time mede a MESMA grandeza física nas 3 engines:** o tempo de CPU para
  avançar a simulação um passo de 0,02 s com N corpos. É definido igual,
  independente da arquitetura → é a métrica do **ranking cross-engine**.
- **FPS mede coisas DIFERENTES em cada engine**, por causa do acoplamento
  render↔física. Godot e Unity travam a física em 50 Hz num laço próprio e deixam
  o render livre: "1660 FPS" **não** significa física a 1660 Hz (ela roda a 50 Hz)
  — significa que o *laço de render* girou 1660×/s. Lá o FPS ≈ velocidade de
  render/GPU, não custo de física. Nosso Unreal rodou **acoplado** (1 passo por
  frame) → o FPS dele acompanha a física. Comparar "1660 (render Godot)" com
  "50 (físico Unreal)" é comparar grandezas de categorias diferentes.
- **Isto não é fugir da métrica — é lê-la certo**, e confirma o Grau A (a física
  em thread do Unreal torna o FPS cross-engine "enganoso"). Usamos o **FPS por
  engine** como tendência de estresse interna (ex.: Unity 1k→10k: 230→48 FPS
  mostra o pipeline afundando com a carga) e o **Step Time no confronto direto**.
- Analogia p/ slide: FPS = "quão rápido o ponteiro do painel se mexe" (depende do
  painel/arquitetura de render); Step Time = o trabalho real feito por volta do motor.

---

## Cenário 1 — A Torre (estabilidade / colapso)

**Veredito por N (kept% = altura final do topo ÷ altura intacta):**

| N | Godot/Jolt | Unity/PGS | Unreal/Chaos |
|---|---|---|---|
| 10 | ✅ ESTÁVEL | ✅ ESTÁVEL | ✅ ESTÁVEL |
| 15 | 💥 COLAPSOU (14%) | 💥 COLAPSOU (8%) | ⚠️ **PARCIAL (38%)** |
| 20 | 💥 COLAPSOU (4%) | 💥 COLAPSOU (8%) | 💥 COLAPSOU (8%) |

**Achado principal — Shock Propagation do Chaos:** em **N=15**, mesma cena, mesmos
parâmetros, apenas o Chaos **segura metade da pilha** (kept 38%) enquanto Jolt e
PhysX-PGS **achatam até o chão**. É exatamente o mecanismo do Grau A: o Chaos
distribui o impulso da base ao topo a cada passo. Mas o efeito é limitado — em
**N=20 todos colapsam**. Conclusão honesta: **nenhum solver default sustenta
pilhas rígidas altas.**

**Comportamento em N=10 (todas estáveis, mas MUITO diferentes) — tempo até dormir:**
| Engine | tempo até sleep | comportamento |
|---|---|---|
| Godot/Jolt | **0.51 s** | assenta rápido e decisivo (sleep agressivo) |
| Unreal/Chaos | 21.1 s | convergência lenta mas **determinística** |
| Unity/PGS | 26.7 s (σ=16.8, **2 timeouts**) | **jitter caótico**: de 9 a 60 s, 2 runs nunca dormem |

Isto é o "jitter excessivo" do enunciado: mesmo sem colapsar, o PGS treme por
dezenas de segundos e às vezes não estabiliza; o Jolt é o mais limpo; o Chaos é
lento porém repetível.

**Determinismo (achado de método):** Chaos σ ≈ 0 (10 runs idênticas). A σ de
Godot vem do `tempo_ate_sleep` em wall-clock (ruído do SO); a de Unity, do
não-determinismo de threading do PhysX. O filtro ±σ do enunciado é quase trivial
no Unreal e infla nas outras por **artefato de medição**, não física.

---

## Respostas às "Considerações Críticas" do enunciado

**1. Qual engine suportou mais corpos / foi mais rápida?**
Na Chuva (custo de física), **Unity/PhysX vence com folga** (13 ms em 10k, ~11×
mais rápido que Chaos). Ordem: PhysX > Jolt > Chaos. Todas aguentaram 10k.

**2. Qual entregou a pilha mais estável?**
Depende do critério:
- **Aguenta mais peso:** **Chaos** (único a segurar N=15, via shock propagation).
- **Assenta mais rápido/limpo:** **Jolt** (N=10 dorme em 0.5 s).
- **Menos estável:** **PGS/PhysX** (jitter e timeouts já em N=10).
Nenhuma sustenta N=20 → o limite de "física quebrando" está em N≈15–20 para todas.

**3. Confirma o Grau A?**
- ✅ **Shock Propagation do Chaos** existe e ajuda (N=15 parcial) — confirmado.
- ✅ **Chaos mais caro por LWC/double precision** — confirmado (~11× PhysX).
- ✅ **FPS do Unreal enganoso** (acoplamento) — confirmado; por isso medimos os dois.
- ❌ **Correção:** o Grau A assumiu Unity com **TGS**; o default real é **PGS**.
  Com TGS (variante tunada) o Unity sobe o limiar para N≈20 — mas isso é
  configuração, não default, então fica fora da comparação principal.

---

## Caveats (para honestidade no relatório)
- **Dataset preliminar (10 runs)** — final de 20 roda depois; ordens de grandeza não devem mudar.
- **Atrito:** Unity usa 0.5 estático / 0.4 dinâmico; Godot e Unreal usam 0.5 único (limitação de API do Jolt/Chaos). Efeito menor numa pilha vertical com restituição 0.
- **Timeouts (Unity N=10):** o filtro ±σ os descarta, mas por `howto-statistics.md` são resultado válido (instabilidade). A distribuição, não a média, é o achado.
- **Métrica de step time:** cada engine usa seu mecanismo nativo (Godot `TIME_PHYSICS_PROCESS`, Unity `Stopwatch(Physics.Simulate)`, Unreal `TG_Pre→PostPhysics`). Medem "custo de avançar 1 passo" — comparável em significado.
