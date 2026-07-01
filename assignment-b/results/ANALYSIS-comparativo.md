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

**Por que o Chaos é tão mais lento?** Coerente com o Grau A: Chaos usa
**double precision (LWC)** e um pipeline mais pesado; aqui rodou **acoplado** ao
frame (para medir o passo). Não é ineficiência de algoritmo isolada — é custo
arquitetural por corpo.

### FPS — ⚠️ NÃO comparar diretamente
| N | Godot | Unity | Unreal |
|---|---|---|---|
| 1k | ~1660 | 230 | 50 |
| 5k | ~430 | 123 | 14 |
| 10k | ~10–80 (bimodal) | 48 | 6.6 |

Godot e Unity **desacoplam** render da física (FPS alto). Unreal rodou
**acoplado** → FPS acompanha o custo da física. Logo o FPS reflete **arquitetura
de threading**, não velocidade de física. **Step Time é a métrica honesta.**

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
