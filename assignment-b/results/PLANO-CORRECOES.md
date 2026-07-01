# PLANO DE CORREÇÕES — Grau A + Grau B

> Arquivo de continuidade. Deriva de `AUDITORIA.md`. Ataca os achados **um de
> cada vez**. Cada tarefa tem responsável (👤 Dono = precisa rodar engine / decidir;
> 🤖 Claude = edição de doc/dado/script), dependências e critério de conclusão.
> Atualizar o **Status** ao fechar cada tarefa.
>
> Legenda de status: ⬜ a fazer · 🔄 em andamento · ✅ feito · ⏸️ bloqueado (aguarda decisão)

---

## Decisões do dono (2026-07-01) — registradas

| Item da auditoria | Decisão do dono | Efeito no plano |
|---|---|---|
| **1 — dados obsoletos** | "Rodei cada benchmark ontem, **10 iterações de cada**. Usar só os últimos corretos." | Dataset **final = 10 iterações** (resolve a dúvida 10/20/30 → **é 10**). Canônico = batch de ontem (sim-time). Obsoletos saem do comparativo. → **T1** |
| **2 — métrica da Torre** | "A torre devia ser igual pra todos; o cálculo não devia ser baseado no tempo até o jitter finalizar." | Trocar métrica primária da Torre de *tempo-até-sleep* para *colapso/estabilidade*. Revisão abaixo confirma que **faz sentido**. → **T3** |
| **3 — Torre N=100** | "A torre 100 realmente foi testada nas outras? O que precisa ser feito?" | Resposta abaixo. Godot e Unity têm N=100; **Unreal não**. → **T4** |
| **4 — PGS vs TGS** | "Corrija, usamos o default." | Corrigir Grau A: default do Unity = **PGS**. → **T2 (✅ feito neste turno)** |
| **5 — gap do Chaos** | "Não entendi." | Explicação em linguagem simples abaixo (seção "Item 5 explicado"). → **T6** |

---

## Mapa canônico de datasets (revisão do item 1 — CONCLUÍDA)

Verificado por data de commit no Git + `amostras` nos CSVs. **"Canônico" = batch de ontem, sim-time, 10 runs. Obsoleto = wall-clock antigo.**

### Cenário 2 — Chuva (métrica: sim-time, `amostras=500`, 10 runs)

| Engine | ✅ USAR (canônico) | ❌ Obsoleto (arquivar/ignorar) | Como sei que é obsoleto |
|---|---|---|---|
| Godot | `chuva-default/` (commit 07-01, 30 linhas, amostras 500) | `chuva-default-buffer/`, `chuva-tuned-buffer/`, `chuva-tuned-partial/` (commit 06-30) | amostras **< 500** no 10k (418/364/333) = janela wall-clock antiga |
| Unity | `chuva-default/` (07-01, 30 linhas, amostras 500) | — | é o único |
| Unreal | `chuva-default/` (07-01, 30 linhas, amostras 500) | — | é o único |
| Unreal (exercício à parte) | `chuva-optimized/` (07-01) | *não é obsoleto* — é o experimento "otimização que piorou", **não entra na coluna principal** | por design |

> ✔️ **Boa notícia:** o `ANALYSIS-comparativo.md` **já usa os canônicos** (Godot
> 2.87/15.13/32.87 = `chuva-default`). O problema é só que (a) o `RESULTS.md`
> ainda **lista** os obsoletos junto, e (b) `godot/NOTES.md` + `HANDOFF.md`
> ainda citam os números obsoletos (2.62/13.39/31.66) como "canônicos". → T1 e T5.

### Cenário 1 — Torre (métrica muda para colapso — ver T3; 10 runs)

| Engine | ✅ USAR (canônico) | ❌ Exploratório/obsoleto | Observação |
|---|---|---|---|
| Godot | `torre-sweep/` (07-01, N=10/15/20 ×10) **+** `torre-N100-arena/` (06-30, N=100 ×10) | `torre-N100/` (sem paredes), `torre-N10/`, `torre-N25/` (gap 1.02) | N=100 é de anteontem mas é válido (10 runs, arena) |
| Unity | `torre-sweep/` (07-01, N=10/15/20 ×10) **+** `torre-N100-arena/` (06-30, N=100 ×10) | `torre-sweep-default/tgs/tuned/` (n=3), `torre-N100-tuned/`, `torre-N100-tgs/` (vazio) | PGS/TGS/+iter = "análise crítica" opcional; **n=3 é insuficiente** (CR-06) |
| Unreal | `torre-sweep/` (07-01, N=10/15/20 ×10) | `torre-N100-arena/` (**vazio — a coletar**) | **falta N=100** → T4 |

---

## Item 5 explicado (o que você não entendeu)

**A afirmação problemática:** os documentos dizem que o Chaos (Unreal) é ~11×
mais lento que o Unity no 10k, e tentam explicar *por quê*. Uma das razões que
eles dão é: *"o Chaos faz 8 iterações de solver e o PhysX faz 4-5, por isso o
Chaos trabalha mais."* (`ANALYSIS-comparativo.md:42`)

**Por que isso é um erro de raciocínio (auto-contradição):** no MESMO relatório,
vocês rodaram o experimento "Chaos otimizado", onde **reduziram** as iterações do
Chaos de 8 → 4. Se "8 iterações" fosse a causa da lentidão, cortar pela metade
deveria **acelerar**. Aconteceu o **oposto: ficou mais lento** (+16% no 10k). A
conclusão de vocês desse experimento foi: *"o gargalo NÃO são as iterações, é o
pipeline de colisão."* (`ANALYSIS-comparativo.md:65-90`)

**Ou seja:** um trecho diz "iterações são a causa" e outro diz "iterações não são
a causa". **Não podem os dois estarem certos.** Um avaliador atento pega isso.

**Ainda por cima:** o número "PhysX faz 4-5 iterações" está errado — a config do
Unity mostra **6** iterações de posição + 1 de velocidade
(`DynamicsManager.asset:13-14`).

**A correção (T6):** remover "iterações" da lista de causas do gap (o próprio
experimento de vocês refuta), corrigir "4-5" → "6", e apresentar o 11× como
"custo bruto no nosso setup (editor + física acoplada + sync de 10k atores)",
separando o que foi **medido** do que é **conjectura** (LWC, double precision).
Isso deixa o achado "a otimização piorou" **mais** forte, não mais fraco.

---

## Revisão do item 2 (você pediu "revise se faz sentido") — SIM, faz

**Seu instinto está correto.** Basear a comparação da Torre no "tempo até o jitter
parar (sleep)" é problemático porque **cada engine decide que 'dormiu' a uma
velocidade diferente**:

- Unity: dorme abaixo de **0.005 m/s** (`DynamicsManager.asset:11`)
- Jolt/Godot: dorme abaixo de **0.03 m/s** (6× mais frouxo)
- Chaos/Unreal: dorme por **contagem de frames** (5 frames), nem é velocidade

Com limiares diferentes, "Unity demora mais que Godot" pode ser só porque o Unity
é 6× mais exigente pra declarar sleep — **não é física, é definição**. Comparar
esse tempo 1-pra-1 é injusto (é o achado CR-01 da auditoria).

**O que fazer (T3):** a métrica primária da Torre passa a ser **colapso /
estabilidade**, que é geométrica e **independe do limiar de sleep**:
- `kept%` = altura final do topo ÷ altura se intacta (já calculado no `RESULTS.md`)
- **Limiar de colapso**: em que N a pilha deixa de se sustentar (do sweep 10/15/20)
- Veredito ESTÁVEL / PARCIAL / COLAPSOU

O "tempo até sleep" vira **secundário/qualitativo por engine**, sempre com a
ressalva do limiar. Assim a Torre fica "igual pra todos" de fato. **Nenhuma
re-coleta é necessária** para isso — os dados de colapso já existem.

---

## Resposta ao item 3 (Torre N=100 foi testada nas outras?)

| Engine | Torre N=100? | Evidência |
|---|---|---|
| Godot | ✅ **Sim**, 10 runs | `torre-N100-arena/torre_godot.csv` (10 linhas, colapsa 10/10) |
| Unity | ✅ **Sim**, 10 runs | `torre-N100-arena/torre_unity.csv` (10 linhas, colapsa 10/10) |
| Unreal | ❌ **Não** | `torre-N100-arena/torre_unreal.csv` está **vazio** (só cabeçalho) |

**O que precisa ser feito (T4) — duas opções:**
- **Opção A (recomendada, sem re-rodar):** com a métrica nova (colapso, item 2), a
  comparação central é no **sweep N=10/15/20**, que as **3 engines têm**. O N=100
  vira confirmação secundária: Godot e Unity colapsam 10/10; o Unreal já colapsa
  em N=20, então N=100 colapsaria também (declarar como inferência, não medição).
- **Opção B (paridade total):** 👤 você roda a Torre N=100 no Unreal (10 iterações,
  mesma arena) e aí os três têm N=100 medido. Custa uma sessão de Unreal.

👉 **Preciso da sua escolha (A ou B)** para fechar T4.

---

## Tarefas (sequenciadas — atacar de cima para baixo)

### T1 — Consolidar Chuva canônica e regenerar RESULTS.md 🤖 ⏸️
- **Objetivo:** `RESULTS.md` mostrar só os datasets canônicos (batch de ontem); obsoletos fora do comparativo.
- **Ação:** ou (a) mover `godot/{chuva-default-buffer,chuva-tuned-buffer,chuva-tuned-partial}` para um `_arquivo-obsoleto/` (preserva a ilustração de "crash mode"), ou (b) adicionar whitelist no `aggregate_stats.py`. Depois `python aggregate_stats.py`.
- **Depende de:** 👤 **decisão: arquivar ou apagar os obsoletos?** (recomendo arquivar — o Godot NOTES os usa no slide "modo de falha/crash").
- **Done quando:** `RESULTS.md` só tem 1 linha de Chuva por engine/variação (+ `chuva-optimized` claramente rotulado como exercício).
- **Status:** ⏸️ aguarda decisão arquivar/apagar.

### T2 — Corrigir PGS ≠ TGS no Grau A 🤖 ✅
- **Objetivo:** Grau A parar de afirmar que o Unity usa TGS (default real = PGS).
- **Feito neste turno:** `scenario-1-torre-unity.md` (linhas 11 e 14), `references-unity.md:131`, `comparative-table.md` (célula do solver Unity). Correções citam `m_SolverType:0`.
- **Status:** ✅ feito (ver commit desta sessão).

### T3 — Redefinir métrica da Torre (colapso primário, sleep secundário) 🤖 ⬜
- **Objetivo:** implementar a decisão do item 2 no `ANALYSIS-comparativo.md`.
- **Ação:** reescrever a seção Torre para liderar com colapso/`kept%`/limiar; rebaixar tempo-até-sleep a observação secundária com ressalva de limiar; remover comparação direta de tempo 0.51 vs 21 vs 26.7 como se fosse a mesma grandeza.
- **Depende de:** nada (dados existem).
- **Done quando:** a Torre no comparativo é lida por colapso, não por tempo-até-sleep.
- **Status:** ⬜ pronto para começar após seu ok.

### T4 — Torre N=100 no Unreal (Opção A ou B) 👤/🤖 ⏸️
- **Depende de:** 👤 escolha A (reenquadrar, 🤖) ou B (você roda Unreal N=100).
- **Status:** ⏸️ aguarda escolha A/B.

### T5 — Alinhar NOTES/HANDOFF/STATUS com a realidade 🤖 ⬜
- **Objetivo:** eliminar contradições de documentação (auditoria CR-02, CR-06 staleness).
- **Ação:** `godot/NOTES.md` + `HANDOFF.md` → apontar `chuva-default` como canônico (não os buffers); `unity/NOTES.md` → remover "Chuva inválida/step 0.0" (foi re-coletada, é válida); `STATUS.md` → trocar "0% implementação" por estado real; fixar "final = 10 runs".
- **Status:** ⬜.

### T6 — Corrigir explicação do gap do Chaos 🤖 ⬜
- **Objetivo:** remover a auto-contradição das iterações (item 5 acima).
- **Ação:** `ANALYSIS-comparativo.md:42` — tirar "8 vs 4-5 iterações" das causas; corrigir "PhysX 4-5" → "6+1"; separar medido de conjectura no gap 11×.
- **Status:** ⬜.

### T7 — Corrigir "sublinear = vantagem do Jolt" 🤖 ⬜
- **Objetivo:** `godot/ANALYSIS.md` reconhecer que Unity (1.89×) e Unreal (2.28×) **também** são sublineares 5k→10k → é efeito geral, não mérito do Jolt.
- **Status:** ⬜.

### T8 — Ressalva de filtro em bimodal/timeouts 🤖 ⬜
- **Objetivo:** onde há timeout/bimodalidade, reportar mediana + contagem de timeouts (não a Média Final filtrada). Remover/rotular a linha `godot·torre-N100` (final 45.5s sem sentido) no `RESULTS.md`.
- **Status:** ⬜.

### T9 — Atualizar SCENE_SPEC com desvios reais 🤖 ⬜
- **Objetivo:** `SCENE_SPEC.md` refletir o que rodou (atrito 0.5, restituição chuva 0.3, esfera, caixa fechada, grid 3D, sleep default) numa seção "Desvios e porquê".
- **Status:** ⬜.

### T10 — Fontes e coerência fina (baixa prioridade) 🤖/👤 ⬜
- Citações não-verificáveis (`references-academic.md` URLs genéricas); versão Godot 4.6 (Grau A) vs 4.7 (benchmark); gravidade −9.80/−9.81/−9.8. Decidir padronizar ou anotar.
- **Status:** ⬜.

---

## Decisões pendentes que travam tarefas (me responda)
1. **T1:** obsoletos do Godot Chuva — **arquivar** (recomendo) ou **apagar**?
2. **T4:** Torre N=100 Unreal — **Opção A** (reenquadrar, sem rodar) ou **B** (você roda)?
3. **Ordem:** posso seguir com T3, T5, T6, T7, T8, T9 (todas 🤖, não precisam de re-coleta) enquanto você decide 1 e 2?
