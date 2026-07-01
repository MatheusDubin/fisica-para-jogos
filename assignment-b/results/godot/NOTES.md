# Godot — Notas de Metodologia e Decisões

> Diário de bordo da implementação Godot. Documenta cada decisão de setup,
> cada variação testada, e cada deviação intencional para justificar resultados
> no relatório final.

> 🔴 **CORREÇÃO 2026-07-01 (canônico da Chuva mudou):** este diário aponta
> `chuva-default-buffer`/`chuva-tuned-buffer` como canônicos (2.62/13.39/31.66 ms).
> **Isso está OBSOLETO.** Aqueles datasets usavam janela **wall-clock** e foram
> **arquivados** em `../_arquivo-obsoleto/godot/` (item CR-02 da `../AUDITORIA.md`).
> O **canônico atual** da Chuva é **`chuva-default/`** (janela **sim-time**,
> `amostras=500`, 10 runs): **2.87 / 15.13 / 32.87 ms**. Ver `../RESULTS.md` e
> `../ANALYSIS-comparativo.md`. A **Torre** (`torre-N100-arena`, `torre-sweep`)
> permanece válida.

---

## 1. Ambiente

| Item | Valor |
|---|---|
| Engine | Godot 4.7 stable (official, build 5b4e0cb0f) |
| Renderer | Forward+ (D3D12) |
| Physics Engine | **Jolt Physics** (padrão do Godot 4.5+) |
| Linguagem | GDScript |
| Hardware | NVIDIA RTX 4050 Laptop GPU |

### Project Settings

| Setting | Valor | Justificativa |
|---|---|---|
| `physics/common/physics_ticks_per_second` | **50** | Spec do assignment (50Hz / 0.02s) |
| `physics/3d/physics_engine` | `Jolt Physics` | Padrão do Godot moderno; foco do Grau A |
| `display/window/vsync/vsync_mode` | `Disabled` (0) | Spec: V-Sync mascara custo de física |
| `physics/jolt_physics_3d/limits/max_contact_constraints` | **262144** | Tunado (default 20480 estourou em N=10k esferas) |
| `physics/jolt_physics_3d/limits/max_body_pairs` | **262144** | Mesma razão — defaults eram baixos demais |
| `physics/jolt_physics_3d/limits/temporary_memory_buffer_size` | **256** (MiB) | Default 32 MiB estourou na 2ª run de 10k (alocador caía em fallback lento) |

**Defaults Jolt que NÃO foram alterados** (parte do que estamos medindo):
- Gravity: 9.8 m/s² (y eixo negativo)
- Sleep velocity threshold: 0.03 m/s (Jolt) / 0.1 m/s (Godot generic)
- Sleep time threshold: 0.5 s
- Solver iterations: default
- Contact margin: default (~5 mm)
- Sem warm starting de island (característica arquitetural do Jolt)

---

## 2. Cenário 1 — A Torre

### Setup canônico (deliverable)

| Parâmetro | Valor | Conformidade com spec |
|---|---|---|
| Número de cubos | **100** | "quantidade elevada" ✓ |
| Dimensões | 1 m × 1 m × 1 m | Primitiva simples ✓ |
| Espaçamento | 1.00 (exato — faces tocando) | "perfeitamente empilhadas" ✓ |
| Offset inicial do chão | 0.5 (base toca o chão exatamente) | "perfeitamente empilhadas" ✓ |
| Massa | 1 kg | Uniforme ✓ |
| Atrito | 0.5 (cubo e chão) | Igual em todas as engines |
| Bounce (restituição) | 0.0 | Spec recomenda |
| Timeout por execução | 60 s | Spec recomenda |
| Execuções por configuração | 10 | Spec exige |

### Métricas coletadas

| Métrica | Origem | Spec exige? |
|---|---|---|
| `tempo_ate_sleep_s` | `Time.get_ticks_msec()` entre _ready e all-sleep | **SIM** |
| `timeout` (flag) | `elapsed >= 60s` antes de all-sleep | implícito |
| `phys_frames` | `Engine.get_physics_frames()` delta | bonus (validar wall-clock vs sim time) |
| `max_v` | max de `linear_velocity.length()` em todos os corpos | bonus (detectar colapso) |
| `t_primeiro_sleep` / `t_metade_sleep` | timestamps quando 1/50% dormem | bonus (perfil de convergência) |

CSV de diagnóstico por physics frame (`torre_godot_debug.csv`): `run, t_s, phys_frame, asleep, active_objs, max_v, mean_v, top_y`. Usado para identificar colapso (top_y caindo) vs jitter (max_v oscilando).

### Variações testadas

| N | Spacing | Arena | Resultado | Pasta |
|---|---|---|---|---|
| **10** | 1.02 (gap 2cm) | chão 60×60 só | 0 timeouts, variação alta (2.1s–43.5s). Pilha não colapsa. Variância vem de Jolt sem warm-starting. | `torre-N10/` |
| **25** | 1.02 (gap 2cm) | chão 60×60 só | 1 timeout (10%). Algumas runs com colapso parcial. Filtered mean 11.6s. | `torre-N25/` |
| 100 | 1.00 (exato) | chão 60×60 só (sem paredes) | 10/10 colapsaram, **5/10 timeout** por cubos escapando do chão a ~97 m/s. **Histórico** — substituído pela versão com arena. | `torre-N100/` |
| **100** | **1.00 (exato)** | **arena fechada 60×60×150m** | **DELIVERABLE.** 10/10 colapsaram (pilha não sustenta), **0/10 timeout** — paredes contêm cubos ejetados. Mean=8.89s, σ=0.50s, filtered mean=8.83s. Max velocity uniforme em ~35.9 m/s indica modo de falha reproduzível. | `torre-N100-arena/` |

**Por que mantemos N=10 e N=25 no repo:** evidência de que o comportamento muda
com a escala — Jolt aguenta pilhas pequenas com variância de sleep, mas falha
completamente em pilhas perfeitamente empilhadas grandes. Vai como apêndice no
slide de Considerações Críticas.

### Stress-test (bonus) — discontinuado

Iniciamos um sweep de 5 em 5 (N=5,10,15,...,50) com arena fechada para mapear
o threshold de colapso do Jolt. **Interrompido em N=15** ao observar colapso
visual já nessa escala. Decisão:

- O spec do assignment não exige sweep de N. Exige um N fixo (sugere ~100) e
  diz explicitamente que **timeouts são dado válido** (§169 do generic doc).
- O dado canônico `torre-N100/` já demonstra empiricamente que Jolt-defaults
  não sustenta pilhas grandes — finding cumprido.
- A observação qualitativa "colapso visível já em N=15" entra como nota nos
  slides de Considerações Críticas. Não há CSV completo do sweep.

### O que consideramos mas rejeitamos: tuning de solver

Cogitamos aumentar `jolt_physics_3d/simulation/velocity_steps` e
`position_steps` para ver se Jolt sustentaria N≥100. **Rejeitado** porque o
spec do cenário (§2.1 do generic doc) lista explicitamente:

> | Iterações do solver | Padrão da engine | **Não modificar — é parte do que estamos comparando** |
> | Sleep threshold     | Padrão da engine | **Não modificar — é parte do que estamos comparando** |

Mudar essas configurações invalidaria a comparação cross-engine. O dado
honesto é "Jolt out-of-the-box colapsa em N=100" — exatamente o que o spec
prevê (§14 do godot doc: "Observe isso empiricamente").

### Mudanças que fizemos durante exploração (e depois revertemos)

| Mudança experimental | Por que tentamos | Por que revertemos |
|---|---|---|
| `CUBE_SPACING = 1.02` (gap 2cm) | Evitar penetração inicial na margem de contato do Jolt | Viola spec "perfeitamente empilhadas"; reverteu para 1.00 |
| `OFFSET_INICIAL = 0.51` (gap 1cm chão) | Mesma razão | Reverteu para 0.5 |
| `TORRE_NUM_CUBOS = 10` (depois 25) | Reduzir variância para investigar | Spec exige "quantidade elevada"; voltou para 100 |
| Lost-cube detection (cubos abaixo de y=-50 considerados perdidos) | Atalho para sair do timeout quando cubos escapam do chão | Substituído por paredes (solução correta); fugia de uma medida real |

### Arena fechada — decisão revisada: USAR em todas as engines

**Decisão final:** o Torre usa **arena fechada (60×60m chão + 4 paredes +
teto, altura 150m)** em **todas as engines**. Foi tomada depois de revisar
os docs de Unity e Unreal e ver que sem padronização cross-engine, qualquer
engine que ejetasse cubos para fora do chão teria timeouts artificiais
(cubos perdidos no infinito) enquanto outras engines não teriam — comparação
inválida.

Spec (§2.2 generic) só prescreve "chão estático". A arena é uma adaptação
de implementação **acima do spec** que mantemos consistente cross-engine.
Documentar no relatório como tal.

**Por que não influencia a fase inicial:** paredes estão a 30m do centro
da pilha. Para uma pilha estável, os cubos nunca tocam paredes. Só entram
em ação se o solver falhar e ejetar cubos — exatamente o modo de falha que
queremos quantificar de forma uniforme.

### Estado do dataset Godot vs decisão

| Dataset | Setup | Status |
|---|---|---|
| `torre-N100/` | **Sem paredes** (chão 60×60m só) | **Histórico** — coletado antes da decisão de arena. Útil só para mostrar o modo de falha "rogue cubes" no slide de Considerações Críticas. **Não usar no comparativo cross-engine.** |
| `torre-N100-arena/` | **Com arena** (60×60×150m) | **CANÔNICO** ✅ — coletado. 10 runs, 0 timeouts, mean 8.89s (σ 0.50s). |

### Análise do dataset canônico (`torre-N100-arena/`)

**Dados brutos:**

| Run | Tempo (s) | Timeout | max_v (m/s) |
|---|---|---|---|
| 1  | 10.049 | ✗ | 36.30 |
| 2  | 8.977  | ✗ | 35.61 |
| 3  | 8.637  | ✗ | 36.62 |
| 4  | 8.677  | ✗ | 36.43 |
| 5  | 8.696  | ✗ | 35.93 |
| 6  | 8.917  | ✗ | 35.54 |
| 7  | 8.577  | ✗ | 36.33 |
| 8  | 9.417  | ✗ | 35.68 |
| 9  | 8.217  | ✗ | 35.47 |
| 10 | 8.717  | ✗ | 35.58 |

**Estatísticas** (verificadas via cálculo em `ANALYSIS.md`):
- Mean: **8.888s**
- Desvio padrão (σ, população): **0.486s**
- Coeficiente de variação: 5.5% (distribuição apertada)
- Faixa [M ± σ]: [8.403, 9.374]
- Mantidos: runs 2, 3, 4, 5, 6, 7, 10 (7 runs)
- Excluídos: run 1 (10.049, alto), run 8 (9.417, alto), run 9 (8.217, baixo)
- **Filtered mean: 8.743s** ← número canônico para o comparativo
- Max velocity uniforme em 35.47–36.62 m/s

> Análise estatística completa de todos os datasets em
> `assignment-b/results/godot/ANALYSIS.md` (Torre + Chuva, com filtragem
> M±σ aplicada por configuração).

**O que os dados revelam:**

1. **10/10 colapsaram** — Jolt-defaults realmente não sustenta 100 cubos
   perfeitamente empilhados. Confirma a hipótese do contexto do Grau A
   (ausência de warm starting de island).
2. **max_v reproduzível em ~36 m/s** — mesma velocidade de ejeção em todas
   as runs indica modo de falha determinístico do solver (não ruído de OS).
3. **0 timeouts** — métrica limpa. Arena conteve todos os cubos ejetados.
4. **σ pequeno (0.5s)** — convergência rápida do tempo até "pilha de
   escombros estabilizar". Diferente de uma pilha estável que pode ter
   tempos muito variáveis (vide N=10 com σ alto).

**Para o relatório:** este é o número canônico do Godot/Jolt no Cenário 1.

### Implicação para Unity e Unreal

Replicar a arena fechada com as mesmas dimensões. Está documentado em:
- `scenarios/scenario-1-torre-unity.md` → seção "Implementation locks"
- `scenarios/scenario-1-torre-unreal.md` → seção "Implementation locks"
- `HANDOFF.md` → §6

---

## 3. Cenário 2 — A Chuva

### Setup canônico (deliverable)

| Parâmetro | Valor | Conformidade com spec |
|---|---|---|
| Forma | **Esfera** (`SphereShape3D`, raio 0.5 m) | Primitiva simples ✓ (escolhida no doc genérico por evitar instabilidades box-box) |
| Massa | 1 kg | Uniforme ✓ |
| Atrito | 0.4 | Padrão |
| Bounce | 0.3 | "Um pouco de bounce" — testamos dinâmica, não estabilidade |
| Variações | **1.000, 5.000, 10.000** | Spec exige exatamente estas três |
| Container | Caixa fechada 35×80×35 m (6 paredes estáticas) | Spec aceita "funil ou caixa fechada" |
| Spawn | Grid 3D pré-calculado dentro da caixa, jitter ±3cm com seed | Spawn determinístico mas com leve aleatorização para evitar overlap inicial |
| Aquecimento (warmup) | 0.5 s descartados | Estabilizar antes de coletar |
| Janela de coleta | 10 s | Spec: "primeiros segundos após o spawn" |
| Execuções por variação | 10 | Spec exige |

### Métricas coletadas

| Métrica | Origem | Spec exige? |
|---|---|---|
| `physics_step_ms_medio` | `Performance.get_monitor(TIME_PHYSICS_PROCESS) * 1000` — média de 500 amostras na janela | **SIM** ("tempo computacional médio do passo da física") |
| `fps_medio` | `Engine.get_frames_per_second()` — média na janela | **SIM** ("FPS da simulação sob estresse") |
| `physics_step_ms_max` | máximo da janela | bonus (detectar spikes) |
| `amostras` | contagem de samples na janela | diagnóstico — se < 500, simulação está abaixo de 50Hz (spiral-of-death) |

### Tuning aplicado

| Setting | Default | Tunado | Por quê |
|---|---|---|---|
| `max_contact_constraints` | 20480 | **262144** | Default estourou em N=10k. Erro literal: *"Jolt Physics contact constraint buffer exceeded capacity and contacts were ignored"*. Não é tuning de comportamento, só de memória pre-alocada. |
| `max_body_pairs` | 65536 | **262144** | Mesma razão — pré-alocação para broadphase. |

**Importante para a comparação cross-engine:** Unity e Unreal devem usar configurações análogas (limites de buffer altos o suficiente para não saturar), senão a comparação é injusta.

### Sessões executadas

| Sessão | CHUVA_VARIACOES | Tuning Jolt | Status | Pasta |
|---|---|---|---|---|
| 1 | [1000, 5000, 10000] | tudo default | 1k ✓ / 5k ✓ / 10k **crashed na run 2** (contact buffer overflow) | `chuva-default-buffer/` |
| 2 | [10000] | contacts/pairs = 262144 | Run 1 ✓ (29.5ms/10FPS) / **Run 2 estourou temp memory (32 MiB)** | `chuva-tuned-partial/` |
| 3 | [10000] | contacts/pairs = 262144, **temp_mem = 256 MiB** | Run 1 ✓ (29.1ms/9.4FPS) / **Run 2 crash silencioso** entre `_ready` e primeiro physics frame | (não arquivado — refeito com arquitetura nova) |
| 4 | [10000] | mesmo da sessão 3 + **rain refatorado in-place** (sem scene reload) | (pendente) | `chuva-tuned-buffer/` (a criar) |

### Mudança de arquitetura entre sessão 3 e 4

A sessão 3 confirmou que mesmo com todos os buffers tunados o crash da 2ª run persiste em N=10k. Diagnóstico: o ciclo `get_tree().reload_current_scene()` entre runs cria 10000 RigidBody3D, depois destrói tudo, depois cria mais 10000. Algum estado interno do Jolt (provavelmente o body lifecycle entre frames de reload) acumula e a 2ª run trava antes do primeiro physics step.

**Solução:** o `rain_benchmark.gd` foi reescrito para rodar todas as 10 runs **na mesma instância de cena**:
- Câmera, luz, paredes estáticas: criadas uma única vez no `_ready`.
- Cada run: `queue_free()` em todas as esferas da run anterior, aguarda 2 physics frames + 0.3s, spawna esferas novas.
- O `RunManager.chuva_avancar_sem_reload()` atualiza apenas os contadores sem tocar na scene tree.

Essa é a arquitetura correta para stress tests com alto N — evita o overhead e os bugs do scene reload completo.

### Por que o step time da sessão 2 ficou pior que da sessão 1?

Na sessão 1 (defaults), com `max_contact_constraints = 20480`, Jolt **silenciosamente descartava contatos** acima do limite — o warning aparecia mas o solver pulava restrições. Isso resultava em (a) esferas se interpenetrando, (b) eventual crash, **e (c) tempo de step artificialmente baixo (23.8ms)** porque o solver tinha menos trabalho.

Na sessão 2 (contacts tunados para 262144), o solver processa **todos** os contatos reais — fisicamente correto, mas mais caro: 29.5ms/step. **Este é o número honesto** para reportar; o 23.8ms da sessão 1 era um sub-measurement causado pelo bug do buffer.

Implicação para a comparação: ao reportar o número de Godot em N=10k, usar o valor da sessão 3 (tudo tunado, simulação fisicamente correta). Mencionar a sessão 1 como "comportamento com defaults" no slide de Considerações Críticas.

### Achado importante para slides

A sessão default-buffer documenta o **crash mode** do Godot/Jolt em N=10k: o buffer de contatos pré-alocado satura, contatos são descartados, esferas se interpenetram, e a simulação trava. Esta é a evidência do "limite onde a física quebra" que o spec pede em "Considerações Críticas".

A sessão tuned-buffer dá o número real de Physics Step Time em N=10k, para o gráfico de barras comparativo.

---

## 4. Métricas — checklist contra spec

### Cenário 1 — A Torre

Spec exige:
- ✅ Tempo (em segundos) até repouso total (sleep state) → `tempo_ate_sleep_s`
- ✅ Observar jitter / colapso → `max_v` + `top_y` em `torre_godot_debug.csv`

### Cenário 2 — A Chuva

Spec exige:
- ✅ Physics Step Time médio (em ms ou ns) → `physics_step_ms_medio`
- ✅ FPS sob estresse → `fps_medio`
- ✅ Para cada variação de 1.000, 5.000, 10.000

### Metodologia geral

Spec exige:
- ✅ 10 execuções por configuração
- ✅ Calcular média e desvio padrão
- ✅ Filtrar fora de [M ± σ]
- ✅ Calcular Média Final filtrada
- ✅ Automação via script
- ✅ Builds reproducíveis (cada cena é gerada por código)

**Caveat metodológico**: para o Cenário 1 com N=100, a distribuição é **bimodal**
(cluster ~9s = colapso-com-pile / cluster ~60s = timeout). O filtro M±σ
não funciona bem para bimodal — vai descartar valores normais ou manter outliers
dependendo da composição. Reportamos a média filtrada (per spec) **junto com**
a mediana, a contagem de timeouts (5/10), e o intervalo dos settled-runs
(8.5–9.4 s) para clareza.

---

## 5. Arquivos no repositório

```
results/godot/
├── NOTES.md                            (este arquivo)
├── torre-N10/                          (exploratório, gap 2cm)
│   ├── torre_godot.csv
│   └── torre_godot_debug.csv
├── torre-N25/                          (exploratório, gap 2cm)
│   ├── torre_godot.csv
│   └── torre_godot_debug.csv
├── torre-N100/                         (HISTÓRICO — sem paredes, 5/10 timeouts por rogue cubes)
│   ├── torre_godot.csv
│   └── torre_godot_debug.csv
├── torre-N100-arena/                   (DELIVERABLE — com arena fechada, 0/10 timeouts, mean 8.89s)
│   ├── torre_godot.csv
│   └── torre_godot_debug.csv
├── chuva-default-buffer/               (defaults — captura o crash mode em 10k)
│   └── chuva_godot.csv
├── chuva-tuned-partial/                (intermediário — 1 run completa antes do crash da run 2)
│   └── chuva_godot.csv
└── chuva-tuned-buffer/                 (DELIVERABLE 10k — buffers tunados + in-place reset)
    └── chuva_godot.csv
```

---

## 6. Status final da implementação Godot

| Cenário | Variação | Estado | Pasta |
|---|---|---|---|
| Torre | N=100, sem paredes | 🟡 Histórico — não usar no comparativo | `torre-N100/` |
| **Torre** | **N=100, com arena** | ✅ **CANÔNICO** — 10 runs, 0 timeouts, mean 8.89s | `torre-N100-arena/` |
| Chuva | 1.000 esferas | ✅ Coletado | `chuva-default-buffer/` |
| Chuva | 5.000 esferas | ✅ Coletado | `chuva-default-buffer/` |
| Chuva | 10.000 esferas | ✅ Coletado (com buffers tunados) | `chuva-tuned-buffer/` |

**Implementação Godot 100% completa.** Próximos passos:
1. Unity (ver `HANDOFF.md` na raiz de `assignment-b/`).
2. Unreal.
3. Relatório comparativo final.
