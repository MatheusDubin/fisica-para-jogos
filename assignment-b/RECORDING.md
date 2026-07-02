# Gravação dos vídeos — guia rápido

> Como gravar os dois clipes que os slides do Grau B referenciam:
> **(1) o colapso da Torre (N=100)** e **(2) a Chuva de 10k esferas**.
> As três engines já estão em **MODO VÍDEO** (2 runs por cenário, câmeras enquadradas).
> Reverter para o dataset canônico: ver [§4](#4-voltar-ao-modo-benchmark-canônico).

---

## 1. O que está configurado

São **3 clipes por engine**, todos em **2 runs** (modo vídeo):

| # | Clipe | Config | Mostra |
|---|---|---|---|
| 1 | **Torre SWEEP** | N = 10, 15, 20 | o "degrau": N=10 estável → N=15/20 colapsam |
| 2 | **Torre N=100** | N = 100 | o colapso completo (pancaking) dos slides |
| 3 | **Chuva** | 1k / 5k / 10k | a de 10k é a que "engasga" |

A câmera **se reenquadra sozinha para cada N** (sweep incluso) — cada torre aparece bem
enquadrada. Cada run faz **reset in-place** e repete o evento (2 runs = **2 tomadas**; grave a
1ª, a 2ª é backup). Ao fim a aplicação encerra sozinha.

### Enquadramento das câmeras (já calculado)
FOV vertical **60°** e vista 3/4 (azimute 45°) nas três engines → os clipes casam lado a lado.
Fórmula: para um objeto de altura `H` preencher a fração `f` da altura do frame com FOV `θ`,
`R = (H/2) / tan(f·θ/2)`. Com θ=60°, f≈0,80 → **R ≈ 1,15·H** (diagonal) → **0,81·H por eixo**.

- **Torre:** câmera a `0,81·H` por eixo, altura `0,50·H`, mira em `0,42·H` → a torre inteira
  ocupa ~80% do frame e o colapso perto do chão fica visível.
- **Chuva:** câmera a `37 m` por eixo, altura `26 m`, mira em `y=22 m` → enquadra do chão até o
  topo do bloco de spawn de 10k (~50 m).

---

> ⚠️ **Atenção — gravar SOBRESCREVE os CSVs canônicos.** No Unity e no Unreal, o modo vídeo
> escreve em `torre-N100-arena/`, `torre-sweep/` e `chuva-default/` — os **mesmos** caminhos do
> dataset n=15. Depois de gravar, **restaure os dados canônicos**:
> ```bash
> git checkout -- assignment-b/results/unity assignment-b/results/unreal
> ```
> (O Godot escreve em `%APPDATA%`, então não afeta o repo.)

## 2. Como rodar cada engine

### 🟦 Godot — `Projects/physics` (Godot 4.7)
A Torre alterna pela constante `TORRE_N_VALUES` em `scripts/run_manager.gd`.
1. **Torre SWEEP:** em `run_manager.gd`, `TORRE_N_VALUES = [10, 15, 20]`; abra `scenes/tower.tscn`
   e **F6** (roda N=10 → 15 → 20, 2 runs cada, e encerra).
2. **Torre N=100:** volte `TORRE_N_VALUES = [100]`; `tower.tscn` → **F6**.
3. **Chuva:** abra `scenes/rain.tscn` → **F6** (roda 1k → 5k → 10k, 2 runs cada).
   *(Só a 10k? Troque `CHUVA_VARIACOES` para `[10000]`.)*

### 🟥 Unity — `Projects/physics-unity` (Unity 6000.5)
Tudo por menu — deixe a **Game view** visível (é ela que mostra a câmera do benchmark).
1. **Benchmark → `Video - Torre SWEEP 10,15,20 (2 runs)`**
2. **Benchmark → `Video - Torre N=100 (2 runs)`**
3. **Benchmark → `Video - Chuva 1k 5k 10k (2 runs)`** (ou `Video - Chuva 10k` só a 10k)

Cada item entra em Play sozinho. Os itens numerados (1..5) são o dataset canônico de 15 runs —
**não** use para vídeo.

### 🟪 Unreal — `Projects/physics_unreal 5.8` (UE 5.8)
1. **Recompile primeiro** (editei C++): Build no Rider/VS ou deixe o Live Coding recompilar.
2. Cenário em `Config/DefaultGame.ini` (ou **Project Settings → Game → Benchmark**):
   - **Torre SWEEP:** `Scenario=Tower` + trocar `+TowerN=100` por `+TowerN=10` / `+TowerN=15` / `+TowerN=20`.
   - **Torre N=100:** `Scenario=Tower` + `+TowerN=100`.
   - **Chuva:** `Scenario=Rain` (roda 1k/5k/10k). **Hoje está em `Rain`.**
3. Play (PIE). Para imagem limpa use **Play → Standalone Game** ou **New Editor Window**, e **F11**
   para tela cheia (esconde gizmos do editor).

---

## 3. Dicas de captura

- **Grave em 16:9 (ex.: 1920×1080).** O enquadramento horizontal assume 16:9; numa janela com
  outra proporção o vertical continua certo, mas as bordas laterais mudam.
- Capture com **OBS Studio**, **Xbox Game Bar** (`Win+G`) ou o gravador da própria engine.
- As paredes da arena/caixa são **invisíveis** (só o chão é renderizado) — a imagem já fica limpa.
- **A Chuva 10k roda abaixo do tempo real** (a física "engasga" — é justamente o achado): o clipe
  sai em câmera lenta natural. Isso é esperado e ilustra o ponto do slide.
- A Torre N=100 colapsa e "dorme" em ~10 s de simulação; o clipe é curto.

---

## 4. Voltar ao modo benchmark canônico

Cada arquivo tem um comentário com os valores originais. Resumo:

| Engine | Arquivo | Reverter para |
|---|---|---|
| Godot | `physics/scripts/run_manager.gd` | `TORRE_TOTAL_RUNS=15`, `CHUVA_TOTAL_RUNS=15`, `TORRE_N_VALUES=[100]` ou `[10,15,20]` |
| Unity | (nada a reverter) | use os itens de menu **1..5** (já são 15 runs); os itens **Video -** são à parte |
| Unreal | `physics_unreal 5.8/Config/DefaultGame.ini` | `TowerRunsPerN=15`, `RainRuns=15`, `+TowerN=10/15/20` |

> As câmeras podem ficar como estão — o enquadramento não afeta os números (o benchmark mede
> física, não render). Só os **run counts / N** precisam voltar ao canônico para regravar dados.

---

## 5. Vídeos publicados — ✅ já embutidos nos slides

Os dois vídeos estão no Google Drive e **embutidos** em `slides/grau-b.html` (iframe que toca ao
vivo + botão "▶ abrir no Drive" como fallback no PDF):

| Slide | Arquivo | Link |
|---|---|---|
| **Cenário 1 · Vídeo** (Torre) | `fisica-para-jogos-benchmark-torre.mp4` | https://drive.google.com/file/d/1fY9vaV7UhcjPy45hc_yabpJSLWkheLQt/view |
| **Cenário 2 · Vídeo** (Chuva) | `fisica-para-jogos-benchmark-chuva.mp4` | https://drive.google.com/file/d/18SGmOFEGHypze815It9uT_AnD4cXsoDr/view |

> ⚠️ Os arquivos precisam estar compartilhados como **"qualquer pessoa com o link"** para o iframe
> tocar na apresentação. Para trocar um vídeo, edite o `FILEID` nas tags `<iframe data-src=...>` e
> `<a href=...>` do respectivo slide `... · Vídeo` em `grau-b.html`.
>
> Os slides têm 2 slots (Torre e Chuva). Se quiser exibir o **sweep** à parte, duplique o slide de
> vídeo da Torre e aponte para outro clipe.
