# AUDITORIA — Raciocínio, Teoria e Integridade de Dados (Grau A + Grau B)

> ⚠️ **DOCUMENTO HISTÓRICO — auditoria do piloto n=10, feita ANTES da coleta n=15.**
> Serve como **registro de rigor**: lista as fraquezas do piloto que o dataset canônico
> **n=15 depois corrigiu** — Unreal N=100 (antes ausente, hoje medido: `kept 2%`), PGS≠TGS
> (corrigido), datasets wall-clock obsoletos (separados em `_arquivo-obsoleto/`), N final
> (definido como 15). Os "ERROS COMPROVADOS" abaixo referem-se ao **estado antigo**; a maioria
> **já não se aplica**. Números canônicos: [`RESULTS.md`](RESULTS.md) · leitura: [`README.md`](README.md).

> Revisão técnico-científica cética, anterior à entrega ao professor. Objetivo:
> encontrar falhas de raciocínio, erros teóricos, inconsistências estatísticas e
> alegações não sustentadas pelos dados. **Nenhum arquivo analisado foi alterado**
> (só este documento foi criado). Fase 2 (estruturação de slides) aguarda aval.
>
> **Método de verificação:**
> - Recomputei M, σ (populacional ÷n), faixa [M±σ], descartes e Média Final
>   diretamente dos 35 CSVs brutos, com implementação independente do
>   `aggregate_stats.py`. **Resultado: a aritmética de `RESULTS.md` está correta e
>   reproduz byte-a-byte a partir do script** (uma única linha fora de ordem —
>   item BX-02). O problema **não** é a conta; é *metodologia, comparabilidade,
>   proveniência dos dados e interpretação física*.
> - Confirmei configs de engine lendo `DynamicsManager.asset`, `project.godot`,
>   `DefaultEngine.ini`, e os CSVs de `debug` (top_y, phys_frames).
>
> Legenda de categorias: **A** estatística/integridade · **B** coerência
> metodológica cross-engine · **C** física/interpretação · **D** rastreabilidade/fontes.

---

## 0. O que passou na auditoria (para não jogar fora o que está certo)

| # | Verificação | Veredito |
|---|---|---|
| OK-1 | σ é populacional (÷n / STDEVP), conforme `howto-statistics.md:274` e o enunciado | ✅ Coerente e aplicado uniformemente (`aggregate_stats.py:37`) |
| OK-2 | Aritmética de M, σ, filtro [M±σ] e Média Final em todas as ~40 linhas de `RESULTS.md` | ✅ Recomputação independente bate em todos os números |
| OK-3 | Lógica do filtro (inclusivo nas bordas, ε=1e-9) e agrupamento por N/variação | ✅ Sem off-by-one; sem descarte indevido *na mecânica* |
| OK-4 | Migração wall-clock→sim-time da Chuva (amostras=500) | ✅ Aplicada em Unity, Unreal e no dataset Godot `chuva-default` (ver ressalva BX-04/CR-05) |
| OK-5 | Unity default = **PGS** (`m_SolverType: 0`) | ✅ Achado do Grau B confirmado no `DynamicsManager.asset:37` |

> Ou seja: **os números que o script produz estão corretos**. As falhas abaixo são
> sobre *quais* números entram no comparativo, se são *comparáveis*, e sobre as
> *conclusões* tiradas deles.

---

## 1. ERROS COMPROVADOS (verificados contra dado/config concreto)

Ordenados por severidade.

| ID | Sev. | Cat. | Arquivo:linha | Problema | Evidência concreta | Ação recomendada |
|---|---|---|---|---|---|---|
| **CR-01** | 🔴 Crítica | B | `DynamicsManager.asset:11`; `results/godot/NOTES.md:35`; `results/unreal/NOTES.md:43`; `SCENE_SPEC.md:14` | **O limiar de sleep é DIFERENTE nas 3 engines** e nenhum é o do spec (0.05). Como a métrica primária da Torre é "tempo até sleep", a comparação cross-engine mede coisas diferentes: cada engine declara "dormiu" a uma velocidade distinta. | Unity `m_SleepThreshold = 0.005`; Jolt default `0.03`; Chaos = "disable após 5 frames" (frame-based, sem limiar de velocidade). Unity é **6× mais estrito** que Jolt → tende a acusar mais tempo até sleep **por definição**, não por física. | Reportar o limiar de cada engine ao lado de cada número de tempo-até-sleep; declarar explicitamente que a comparação de tempo-até-sleep é **qualitativa**, não 1-pra-1. Não afirmar "Unity treme mais que Jolt" sem essa ressalva. |
| **CR-02** | 🔴 Crítica | A/B | `results/RESULTS.md` (Cenário 2, seção Physics Step Time); `results/godot/NOTES.md:115,333`; `HANDOFF.md:154-158` | **`RESULTS.md` mistura datasets obsoletos (wall-clock) e novos (sim-time) sem marcar quais**, e os documentos-mestre apontam para o dataset ERRADO como canônico. A mesma config aparece várias vezes com números diferentes. | Godot 10k esferas aparece **4×**: `23.841` (default-buffer, amostras 418, 1 run), `32.868` (default, sim-time, 500), `31.657` (tuned-buffer, amostras 363-367 = **wall-clock**), `29.514` (partial, 333). `chuva-default-buffer`/`chuva-tuned-buffer` têm amostras **< 500** ⇒ são wall-clock OBSOLETOS (por `FOLLOWUP`), mas NOTES/HANDOFF ainda os chamam de "CANÔNICO" (2.620/13.391/31.657). O comparativo usa outro (`chuva-default`: 2.87/15.13/32.87). | Marcar em `RESULTS.md` cada linha como CANÔNICA (sim-time, amostras 500) ou OBSOLETA (wall-clock); remover ou mover as obsoletas para um apêndice. Atualizar `godot/NOTES.md` §5/§6 e `HANDOFF.md` para apontar `chuva-default` como canônico. |
| **CR-03** | 🔴 Crítica | A/B | `results/unreal/torre-sweep/torre_unreal.csv`; `results/unreal/torre-sweep/torre_unreal_debug.csv` | **As "10 runs" do Unreal são bit-idênticas** (n efetivo = 1). O tratamento estatístico exigido (M±σ, filtro) é vazio para o Unreal, e o "determinismo do Chaos" apresentado como achado confunde *ausência de amostragem independente* com *física*. | N=15: as 10 runs têm `tempo=28.1605`, `max_v=19.479887`, `phys_frames=1408` **idênticos**; debug confirma `top_y` final = `5.4941` único nas 10 runs. Idem N=10 (`21.1003`) e N=20 (`8.2600`). σ=0 é estrutural. | Ou (a) introduzir variação real entre runs no Unreal (seed/jitter, igual à Chuva), ou (b) declarar honestamente "Chaos é determinístico ⇒ 1 execução representativa; M±σ não se aplica". Não comparar σ≈0 (Unreal) com σ de Godot/Unity como se medissem a mesma coisa. |
| **CR-04** | 🔴 Crítica | B/C | `results/unreal/torre-N100-arena/torre_unreal.csv` (vazio); `results/ANALYSIS-comparativo.md:126-139` | **Não existe dado de Torre N=100 para o Unreal.** O Unreal só foi testado no sweep N=10/15/20. A "pilha mais estável" (Consideração Crítica obrigatória) nunca foi medida no N=100 que o enunciado sugere, para a engine cuja tese central (Shock Propagation) depende disso. | `torre_unreal.csv` e `torre_unreal_debug.csv` têm **0 linhas de dados** (só cabeçalho). Godot e Unity têm N=100 (10 runs cada); Unreal não. | Coletar Torre N=100 no Unreal (ou declarar explicitamente que a comparação de estabilidade se dá no sweep N=10/15/20 e que N=100 do Unreal é ausente — não inferir). |
| **CR-05** | 🟠 Alta | C | `assignment-b/scenarios/scenario-1-torre-unity.md:11,14`; `assignment-a/references-unity.md:131` | **A afirmação errada "Unity/PhysX default usa TGS" continua viva** fora dos NOTES do Grau B. A correção (default = PGS) só existe nos resultados; o Grau A/cenário não foi corrigido. Pior: as previsões baseadas em TGS são **contraditas pelos dados**. | `scenario-...-unity.md:11`: *"o PhysX 4 usa o solver TGS — não o PGS clássico"*; `:14`: TGS ⇒ "pilhas altas convergem mais suavemente, menos jitter". Realidade (`DynamicsManager.asset:37` = PGS): Unity teve **o pior jitter** (26.7 s, 2 timeouts em N=10 — `ANALYSIS-comparativo.md:147`) e **colapsou** N=100. `comparative-table.md` é *omissa* (nem TGS nem PGS). | Corrigir `scenario-1-torre-unity.md` e `references-unity.md:131` (TGS existe no PhysX 4 como *biblioteca*, mas o **default do Unity é PGS**). Refletir a correção no `comparative-table.md`. Marcar a previsão "TGS ⇒ menos jitter" como refutada. |
| **CR-06** | 🟠 Alta | A | `results/RESULTS.md:22-31` (unity torre-sweep-*); `results/godot/chuva-default-buffer` (10k); `results/godot/chuva-tuned-partial` | **Datasets com n < 10 entram em `RESULTS.md` sem ressalva inline**, violando o mínimo do enunciado (≥10). Alegações-chave dependem deles. | `unity · torre-sweep-{default,tgs,tuned}`: **n=3 por N** (e `tgs N=30`: **n=1**). O limiar de colapso "PGS cai em 15, TGS em 20" (`unity/NOTES.md:199-201`) repousa em **3 runs/N**. Godot `chuva-default-buffer·10000` e `chuva-tuned-partial·10000`: **n=1**. | Coletar ≥10 runs antes de sustentar as afirmações de limiar; enquanto isso, rotular cada linha n<10 como "exploratório, não conforme ao enunciado". |
| **CR-07** | 🟠 Alta | A | `results/RESULTS.md:14` (godot torre-N100 histórico); `assignment-b/howto/howto-statistics.md:258-260` | **O filtro ±σ descarta timeouts (resultado válido) e, em distribuição bimodal, produz Média Final sem significado físico.** | `godot·torre-N100` (histórico, bimodal 5×~8.5s + 5×60s): o filtro **mantém os 5 timeouts de 60 s e descarta as 3 runs válidas de ~8.5 s** → Média Final = **45.516 s** (número sem sentido). `unity·torre-sweep·N=10`: 2 timeouts de 60 s são descartados → final 26.7 s omite a instabilidade. `howto-statistics.md:258` diz explicitamente: timeouts "NÃO descartar. É resultado válido". Faixas com limite inferior **negativo** aparecem (`godot·torre-sweep·N=10` = [-1.607, 6.876]; `godot chuva 10k FPS` = [-1.1, 41.7]) — sinal de que ±σ não se aplica a bimodal. | Em configs bimodais/com timeout, reportar mediana + contagem de timeouts + clusters, e **não** a Média Final filtrada como número principal. Remover a linha `torre-N100` histórica de `RESULTS.md` ou marcá-la "NÃO USAR — filtro inválido em bimodal". |
| **CR-08** | 🟠 Alta | B/D | `SCENE_SPEC.md:30-34,78-96` vs `HANDOFF.md:76-104`; `results/*/NOTES.md` | **O "ground truth" `SCENE_SPEC.md` foi contrariado em vários parâmetros e nunca atualizado.** Quem ler o spec e depois os dados verá contradições. | Spec vs real: atrito estático **0.6→0.5** (chão **0.8→0.5**); restituição da Chuva **0.0→0.3**; forma da Chuva **box→esfera**; container **funil (pirâmide invertida)→caixa fechada**; spawn **queda aleatória→grade 3D**; sleep **0.05→defaults da engine**. Deviações aplicadas *igualmente* nas 3 engines (comparação interna justa), mas o spec declarado ainda diz o contrário. | Atualizar `SCENE_SPEC.md` para refletir o que foi de fato rodado, com uma seção "Desvios do spec original e por quê". Sem isso, o professor pode ler o spec como não cumprido. |
| **CR-09** | 🟡 Média | B | `results/godot/NOTES.md:62` vs `results/unity/NOTES.md:103` e `results/unreal/NOTES.md:149` | **A métrica "tempo até sleep" da Torre é WALL-CLOCK no Godot, mas SIM-TIME nas outras duas.** Metodologia não idêntica; parte do σ (e uma fração dos meios) é artefato. | Godot: `tempo_ate_sleep_s = Time.get_ticks_msec()` (relógio de parede). Confirmação no CSV: `torre-N100-arena` run1 `t=10.049`, `phys_frames=504` → 504×0.02=**10.08 ≠ 10.049**. Unity/Unreal: `t = phys_frames×0.02` exato (Unity run1 `9.78 = 489×0.02`). | Uniformizar: medir tempo-até-sleep em tempo de simulação também no Godot (recontar de `phys_frames`), ou declarar a diferença de relógio ao comparar. |
| **CR-10** | 🟡 Média | C | `results/godot/ANALYSIS.md:206-222` | **A tese "escalonamento sublinear 5k→10k é vantagem do Jolt" é refutada pelos próprios dados das outras engines.** | Sublinear em **todas**: Jolt 15.13→32.87 = **2.17×**; Unity 6.92→13.10 = **1.89×**; Chaos 65.59→149.83 = **2.28×**. Se todas são sublineares, não é característica do Jolt — é efeito comum (janela captura a pilha assentada, mais barata que a broadphase de queda). `godot/ANALYSIS.md:214-221` ainda especula "job system do Jolt / cache locality" como se fosse exclusivo. | Reescrever a seção: o sublinear 5k→10k é **geral** (artefato de janela/regime), não mérito do Jolt. |
| **CR-11** | 🟡 Média | C | `results/ANALYSIS-comparativo.md:42` vs `:49-90`; `DynamicsManager.asset:13` | **Contradição interna na explicação do gap do Chaos** e um número factualmente errado. | `:42` atribui a lentidão do Chaos a "8 iterações de posição (PhysX ~4-5)". Mas (a) o Unity roda **6** iterações de posição + 1 de velocidade (`DynamicsManager.asset:13-14`), não "4-5"; e (b) a própria seção "Chaos-otimizado" (`:49-90`, `unreal/NOTES.md:332-358`) **conclui que iterações NÃO são o gargalo** (cortar 8→4 *piorou*). As duas afirmações não podem coexistir. | Remover "8 vs 4-5 iterações" da lista de causas do gap (o experimento otimizado a refuta). Corrigir "PhysX ~4-5" → "PhysX 6+1 (default Unity)". |
| **CR-12** | 🟢 Baixa | D | `assignment-a/comparative-table.md:10`; `assignment-a/research-godot.md:8`; `results/godot/NOTES.md:13,25` | **Versão pesquisada ≠ versão testada** e data de "Jolt vira default" inconsistente. | Grau A: **Godot 4.6** (Jolt default em "4.6+"). Benchmark: **Godot 4.7** (`godot/NOTES.md:13`), que diz "Jolt default em **4.5+**". `STATUS.md:24` diz "4.6+". Três datas diferentes. | Fixar a versão de fato usada (4.7) em todos os docs, ou re-rodar em 4.6 para casar com o Grau A; unificar a alegação de "quando Jolt virou default". |
| **CR-13** | 🟢 Baixa | D | `assignment-b/FOLLOWUP-...md:52`; `HANDOFF.md:407` vs `results/ANALYSIS-comparativo.md:2,183` | **Tamanho do dataset final inconsistente: 30 vs 20 ciclos.** | FOLLOWUP/HANDOFF/NOTES: "**30 ciclos**". `ANALYSIS-comparativo.md:2` e `:183`: "final de **20**, roda depois". Além disso, **nenhum** dataset atual tem 20 nem 30 — tudo é n≤10 (preliminar). | Decidir e registrar o N final (ver Pergunta P-2). |
| **CR-14** | 🟢 Baixa | D | `SCENE_SPEC.md:12`; `DynamicsManager.asset:7`; `DefaultEngine.ini:273`; `project.godot` (sem gravity) | **Gravidade não idêntica nas 3 engines** (nenhuma = spec −9.81). | Unity −9.81; Unreal `DefaultGravityZ=-980` = **−9.80** m/s²; Godot default = **9.8**. Diferença física desprezível, mas viola "reproduzir identicamente". | Padronizar em −9.81 (Unreal: −981) ou anotar como desvio aceito. |
| **CR-15** | 🟢 Baixa | A | `results/RESULTS.md:13-16` | **`RESULTS.md` está levemente desatualizado vs uma execução limpa do script** (só ordenação). | `diff` da regeneração: a linha `godot·torre-N10` aparece em posição diferente (conteúdo idêntico). Indica que o arquivo commitado não foi regenerado após a última mudança de arquivos. | Rodar `python aggregate_stats.py` e commitar o `RESULTS.md` regenerado. |

---

## 2. SUSPEITAS A VERIFICAR / INTERPRETAÇÕES FRÁGEIS (não são erro de conta — são excesso de conclusão)

| ID | Sev. | Cat. | Arquivo:linha | Suspeita | Por que é frágil | Ação recomendada |
|---|---|---|---|---|---|---|
| **SU-01** | 🟠 Alta | C | `results/ANALYSIS-comparativo.md:134-139`; `results/unreal/NOTES.md:213-227` | **"Shock Propagation explica o N=15 parcial do Chaos"** — causação afirmada, não demonstrada. | O N=15 do Unreal mantém `top_y=5.49` (38%), MAS com `max_v=19.5 m/s` (há ejeção/colapso parcial), i.e., **não** é "segurar metade da pilha", é colapsar para um monte de escombros mais alto. "Segura metade" superestima 38% (é ~⅓). O Shock Propagation nunca foi **desligado** para provar que é ele. | Rebaixar de "é exatamente o mecanismo do Grau A" para "consistente com, porém não isolado". Se quiser causalidade: rodar com o CVar de shock propagation off e mostrar a diferença. |
| **SU-02** | 🟠 Alta | C | `results/ANALYSIS-comparativo.md:26-47`; `assignment-a/research-unreal.md:56` | **O gap de ~11× do Chaos "confirma o Grau A (LWC/double precision)"** — a magnitude e a métrica não sustentam a conclusão limpa. | (a) O próprio Grau A cita Chaos "~54% mais lento" que PhysX (`research-unreal.md:56`, UE5.0). 11× é **7× além** disso ⇒ o medido é dominado por editor/PIE + acoplamento + **sync de transform de 10k atores** (admitido em `ANALYSIS:40`), não pelo solver. (b) A métrica do Unreal (`TG_Pre→PostPhysics`) **não é** semanticamente igual ao `Physics.Simulate()` do Unity — inclui trabalho fora do solver. O "11×" não é comparação de solver limpa. | Apresentar o 11× como "custo bruto do passo no nosso setup acoplado+editor", separando do "custo intrínseco do solver" (não medido). Não vender como confirmação quantitativa do LWC. |
| **SU-03** | 🟡 Média | C | `results/ANALYSIS-comparativo.md:30`; `results/godot/NOTES.md:226-249` | **"As 3 rodaram 10k sem crash"** omite que o Godot/Jolt **crashou** no 10k com buffers default. | Godot precisou `max_contact_constraints 20480→262144`, `max_body_pairs→262144`, `temp_mem 32→256 MiB` **e** reset in-place para não crashar (`NOTES:226-249`). Não há evidência de que Unity/Unreal receberam verificação de buffer equivalente ⇒ possível assimetria cross-engine. | Reescrever para "as 3 completaram 10k **após** ajustes de buffer no Godot". Verificar/registrar os limites internos de Unity (PhysX) e Unreal (Chaos) para simetria. |
| **SU-04** | 🟡 Média | C | `results/RESULTS.md` (Cenário 1) | **Comparar "tempo até sleep" entre regimes distintos** (estável / parcial / colapso) trata grandezas diferentes como a mesma. | Ex.: Unreal N=15 "28.16 s" (parcial, jitter no limiar) vs N=20 "8.26 s" (colapso rápido) — não-monotônico porque um monte que colapsa assenta rápido e uma pilha parcial fica teimando. Comparar esses números como "tempo até sleep" lado a lado engana. | Estratificar: separar tempo-até-sleep de pilhas ESTÁVEIS do tempo-até-assentar de COLAPSOS; não plotar juntos sem rótulo de regime. |
| **SU-05** | 🟢 Baixa | A/C | `results/RESULTS.md` (Chuva Step Time); `FOLLOWUP:34-46` | **A janela sim-time (500 passos) pode subcontar o "estresse de colisão"** que o enunciado pede. | Pela conta do próprio FOLLOWUP, a pilha pousa em ~3,5 s (≈175 passos); os outros ~325/500 passos medem a **pilha já assentada** (mais barata). A média é diluída pela fase barata; o `physics_step_ms_max` (gravado, não reportado) captura melhor o pico. | Reportar também o `physics_step_ms_max` (ou percentil 95) como proxy do estresse de pico, além da média. |
| **SU-06** | 🟢 Baixa | D | `assignment-a/references-academic.md:76,110,57,108` | **Citações acadêmicas não-específicas / autoria ausente.** | Dois papers SIGGRAPH 2025 distintos citam a **mesma** URL genérica (`.../Conference-Papers.html`, linhas 76 e 110); papers de arXiv listados como "Anonymous" (57, 108) embora arXiv tenha autoria. | Substituir por DOIs/links específicos e nomear autores; sem isso, essas linhas são não-verificáveis. |
| **SU-07** | 🟢 Baixa | D | `assignment-a/comparative-table.md:15`; `research-unreal.md:31` | **"Solver base do Chaos = XPBD"** é caracterização comum, mas discutível. | O Chaos usa um solver da família PBD/Gauss-Seidel posicional; rotular categoricamente como "XPBD" é simplificação. Baixo impacto, mas é uma afirmação forte sem citação primária do código. | Suavizar para "solver posicional da família PBD (frequentemente descrito como XPBD)". |

---

## 3. AMBIGUIDADES QUE EXIGEM DECISÃO DO DONO (não assumi — pergunto antes)

| P | Pergunta | Contexto / opções | Impacto |
|---|---|---|---|
| **P-1** | **Qual é o dataset CANÔNICO da Chuva no Godot?** | `godot/NOTES.md`/`HANDOFF` dizem `chuva-default-buffer`+`chuva-tuned-buffer` (2.62/13.39/31.66, **wall-clock**). O `ANALYSIS-comparativo` usa `chuva-default` (2.87/15.13/32.87, **sim-time**). São incompatíveis. | Muda **todos** os números de Godot na Chuva e o ranking cross-engine. |
| **P-2** | **Qual N final: 10, 20 ou 30 ciclos?** E ele será de fato coletado antes da entrega? | FOLLOWUP/HANDOFF dizem 30; `ANALYSIS-comparativo` diz 20; o que existe é n≤10. | Define se os slides usam "preliminar" ou "final" e se as afirmações de limiar (n=3) sobrevivem. |
| **P-3** | **Qual sweep é canônico para a Torre N=10 do Unity?** | O comparativo usa `torre-sweep` (n=10, 26.7 s, 2 timeouts — rotulado "histórico" em `unity/NOTES.md:305`). Existe `torre-sweep-default` (n=3, 10.09 s — rotulado como o "preliminar→30"). Números **contraditórios** e o mais dramático foi o escolhido para a narrativa "PGS jitter caótico". | Risco de leitura de *cherry-picking*; muda a resposta de "qual a mais estável". |
| **P-4** | **Coletar Torre N=100 no Unreal?** (hoje ausente — CR-04) | Sem ele, a comparação de estabilidade no N sugerido pelo enunciado fica incompleta justamente para a engine da tese central. | Define se o slide de estabilidade compara em N=100 ou só no sweep 10/15/20. |

---

## 4. RESUMO EXECUTIVO — os 5 problemas que mais ameaçam a nota

> Ordem = risco à nota. Para cada um: o que muda no entregável se corrigido.

1. **CR-02 — Datasets obsoletos e ponteiro canônico errado (Godot Chuva).**
   `RESULTS.md` exibe wall-clock (obsoleto) e sim-time misturados; NOTES/HANDOFF
   citam os números OBSOLETOS como canônicos. Um avaliador que cruzar NOTES ×
   comparativo encontra **números que não batem** — o pior tipo de erro num
   trabalho de "rigor estatístico". *Corrigindo:* uma única tabela de Chuva por
   engine, marcada sim-time/amostras=500, e NOTES/HANDOFF alinhados. Resolve
   P-1. **É a correção de maior retorno.**

2. **CR-01 + CR-09 — A Torre não é comparável 1-pra-1 como está.** Sleep
   threshold diferente por engine (0.005 vs 0.03 vs frame-based) e tempo medido
   em relógios diferentes (wall-clock no Godot, sim-time nos outros). A
   Consideração Crítica obrigatória "qual entregou a pilha mais estável" está,
   hoje, apoiada numa métrica que mede coisas diferentes. *Corrigindo:*
   uniformizar clock + reportar o limiar de cada engine + rebaixar a comparação
   de tempo-até-sleep para qualitativa. Transforma uma alegação frágil numa
   alegação defensável.

3. **CR-03 + CR-04 — A tese central do Unreal está estatisticamente e
   empiricamente descoberta.** As 10 runs do Unreal são idênticas (n efetivo=1,
   M±σ vazio) e **não há Torre N=100**. A narrativa "Shock Propagation faz o
   Chaos sustentar onde os outros falham" (SU-01) repousa em sweep n=1-efetivo e
   sem o N=100. *Corrigindo:* coletar N=100 no Unreal e/ou declarar honestamente
   o determinismo (1 run representativa) + rebaixar a causalidade do Shock
   Propagation. Protege o achado que o time considera "o mais importante do
   relatório".

4. **CR-05 — Erro teórico TGS≠PGS ainda vivo no Grau A/cenário.** Se os slides
   forem montados a partir de `comparative-table.md`/`scenario-*-unity.md`/
   `references-unity.md`, o erro (Unity usa TGS) vai para a apresentação — e os
   dados o contradizem duplamente (Unity é PGS **e** teve o pior jitter).
   *Corrigindo:* uma frase no Grau A ("default do Unity = PGS; TGS é opt-in") e o
   comparativo passa a mostrar coerência teoria↔dado em vez de contradição.

5. **CR-11 + SU-02 — O gap do Chaos está explicado com conjecturas rotuladas
   como fato, e com uma contradição interna.** Atribuir o 11× a "8 iterações"
   enquanto o próprio experimento otimizado prova que iterações não são o
   gargalo é uma falha de raciocínio que um avaliador atento pega na hora.
   *Corrigindo:* separar medido (custo bruto acoplado+editor) de conjecturado
   (LWC, atores), remover a causa "iterações", corrigir "PhysX 4-5"→"6". O
   achado "otimização que piorou" fica **mais** forte, não mais fraco.

### Nota final de escopo
Nada aqui invalida o trabalho — a implementação nas 3 engines, a coleta e a
mecânica estatística estão **corretas**. O risco é de **apresentação e
interpretação**: números obsoletos convivendo com novos, comparações rotuladas
como 1-pra-1 que não são, e conclusões físicas mais fortes do que os dados
suportam. Os 5 itens acima são majoritariamente **edições de texto/rotulagem** +
2-3 coletas pontuais (Unreal N=100; decidir N final), não um retrabalho.

> **Próximo passo (Fase 2):** aguardando seu aval sobre esta auditoria e as
> respostas de P-1..P-4 antes de propor a estrutura da apresentação e o
> mapeamento slide↔arquivo↔número com as correções aplicadas.
