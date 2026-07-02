# Unreal Benchmark — Setup, Build e Como Rodar

> Projeto convertido para **C++** (UE 5.8 + Chaos). Toda a cena é gerada em
> código — mesma filosofia de Unity/Godot, zero setup manual de nível/Blueprint.
> Ambiente detectado: **VS 2026 Community** (MSVC 14.51) + **Windows SDK 10.0.26100**.

---

## 1. Gerar os arquivos de projeto do Visual Studio

Feche o Unreal Editor. Na raiz do projeto (`Projects/physics_unreal 5.8/`):

- **Opção A (mais fácil):** clique com o botão direito em `physics_unreal.uproject`
  no Explorer → **Generate Visual Studio project files**.
- **Opção B (linha de comando):**
  ```powershell
  & "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" `
    -projectfiles -project="$PWD\physics_unreal.uproject" `
    -game -engine
  ```

Isso cria `physics_unreal.sln` e as pastas `Binaries/`, `Intermediate/`.

## 2. Compilar

- **Opção A (via editor):** dê duplo-clique em `physics_unreal.uproject`. Como
  agora há um módulo C++ sem binário compilado, o editor vai perguntar
  *"missing modules, rebuild?"* → **Yes**. Ele compila e abre.
- **Opção B (Visual Studio):** abra `physics_unreal.sln`, selecione a config
  **Development Editor** / **Win64**, e **Build** (Ctrl+Shift+B). Depois abra o
  `.uproject`.
- **Opção C (linha de comando):**
  ```powershell
  & "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" `
    physics_unrealEditor Win64 Development `
    -project="$PWD\physics_unreal.uproject"
  ```

> **Se o UBT reclamar do compilador VS 2026** ("unsupported compiler version"):
> em `.../UE_5.8/Engine/Saved/UnrealBuildTool/BuildConfiguration.xml` (ou o global
> em `%APPDATA%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`) adicione:
> ```xml
> <Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
>   <WindowsPlatform>
>     <CompilerVersion>14.51.36231</CompilerVersion>
>     <bAllowMSVCUnsupportedCompiler>true</bAllowMSVCUnsupportedCompiler>
>   </WindowsPlatform>
> </Configuration>
> ```

## 3. Preparar o nível (uma vez)

O benchmark é disparado por um **GameMode** (`ABenchmarkGameMode`) já configurado
como default em `DefaultEngine.ini`. Ele funciona em qualquer mapa, mas para um
FPS limpo use um nível **vazio** (sem sky/fog/Lumen do template OpenWorld):

1. **File > New Level > Empty Level**.
2. **File > Save Current Level As** → salve como `main-level` (sobrescreva o
   existente). O `DefaultEngine.ini` já aponta `GameDefaultMap=/Game/main-level`.

Não é preciso colocar nada no nível — arena, corpos, câmera e luz são criados em
código pelo GameMode ao dar Play.

## 4. Escolher o cenário e rodar

**Project Settings > Game > Benchmark** (ou edite `Config/DefaultGame.ini`):

- `Scenario = Tower` → Cenário 1 (Torre). Saída: `results/unreal/torre-N100-arena/`.
- `Scenario = Rain`  → Cenário 2 (Chuva). Saída: `results/unreal/chuva-default/`.

Aperte **Play** (modo **New Editor Window (PIE)** ou **Standalone**). O harness:
roda todas as runs, grava os CSVs e **encerra o Play sozinho** ao terminar.

**Validar a contenção (paredes):** as paredes/teto são collision-only (só o chão
é desenhado, para o FPS ser comparável a Godot/Unity). Para **ver** a arena/caixa
e conferir que os corpos ficam contidos, ligue `bDrawWalls` (Project Settings >
Game > Benchmark). Desligue para os runs oficiais.

**Sweep da Torre (achar onde a pilha colapsa):** troque a lista `TowerN` para
vários valores (ex.: 10,15,20,25,30,50,75,100) — o harness roda cada N em
sequência. Ver `results/unreal/NOTES.md` §3 para o `DefaultGame.ini` pronto.

> Alternativa por linha de comando (sobrepõe o setting):
> adicione `-bench=tower` ou `-bench=rain` aos parâmetros de launch.

## 5. Onde saem os dados

Espelhando Godot/Unity, em `assignment-b/results/unreal/`:

```
torre-N100-arena/
  torre_unreal.csv        run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep
  torre_unreal_debug.csv  run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y
chuva-default/
  chuva_unreal.csv        run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras
```

Velocidades e alturas são gravadas em **m** e **m/s** (convertidas de cm) para
bater 1-pra-1 com Godot/Unity.

## 6. ⚠️ Validação obrigatória na 1ª run (lição do Unity)

No Unity o `physics_step_ms` veio **0.0** por um bug de API e a run foi perdida.
Aqui a métrica vem do timer `TG_PrePhysics → TG_PostPhysics`. **Na primeira run da
Chuva, confira no CSV que `physics_step_ms_medio` NÃO é 0.** Sanity-check vs
Godot: ~2 / ~10 / ~32 ms para 1k / 5k / 10k. Se vier 0, ver a seção
"Métrica de step time" em `results/unreal/NOTES.md`.

## 7. Build Shipping (opcional, para o dataset final)

`Platforms > Windows > Package Project`, com **Build Configuration = Shipping**.
Os números do editor (Development) já são utilizáveis; Shipping remove overhead de
telemetria. Rode o `.exe` empacotado com `-bench=tower` / `-bench=rain`.
