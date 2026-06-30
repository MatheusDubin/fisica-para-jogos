# Cenário 2 — Chuva de Corpos: Godot

> Implementação específica para Godot Engine.
> Base conceitual: `scenario-2-chuva-generic.md`
> Versão de referência: Godot 4.6 (Jolt Physics como padrão)

---

## Contexto do Grau A

O Grau A identificou que Godot 4.6 usa **Jolt Physics** com **job system nativo multi-threaded** que usa todos os cores disponíveis. O que isso significa para a Chuva:

- **Multi-threading nativo é o diferencial aqui:** diferente do GodotPhysics (single-threaded, documentado no Grau A como gargalo acima de ~500 bodies), o Jolt paralela broadphase e solver por design. O Cenário 2 é onde essa diferença deve aparecer com mais clareza — especialmente na variação de 10k objetos.
- **Comparação com Unity (PhysX parcial) e Unreal (Chaos islands):** Jolt usa um job system próprio diferente do approach de islands do Chaos. A escalabilidade entre 1k→5k→10k deve revelar qual modelo de paralelismo é mais eficiente para este tipo de cena de alta dinamicidade.
- **`TIME_PHYSICS_PROCESS` inclui overhead de scheduling:** o Grau A documentou que Godot Physics é acessado via `PhysicsServer3D`. O monitor `TIME_PHYSICS_PROCESS` mede o tempo de wall-clock da fase de física, incluindo o scheduling dos jobs do Jolt. Não é exatamente o mesmo que o Physics Step Time do Unity (Profiler API) ou do Unreal (Insights). Documentar essa diferença metodológica ao apresentar os dados.
- **FPS acoplado ao game loop:** diferente do Unreal (Async Physics Tick), Godot não tem física completamente assíncrona. O `_physics_process` ainda está integrado ao game loop. FPS é uma métrica relevante aqui, como no Unity.
- **Sem GPU physics:** confirmado no Grau A. Todo processamento é CPU, mesmo com Vulkan como backend de rendering.

---

## Configuração do Projeto

- [ ] Fixed Timestep: `Project Settings > Physics > Common > Physics Ticks Per Second = 50`
- [ ] V-Sync: `Project Settings > Display > Window > V-Sync Mode = Disabled`
- [ ] Physics Engine: confirmar `JoltPhysics3D`
- [ ] Gravity: `Project Settings > Physics > 3D > Default Gravity = 9.8`

---

## API de Physics Step Time

Godot expõe o tempo do passo de física via `Performance` singleton:

```gdscript
# Retorna tempo em microssegundos (µs) — converter para ms
func get_physics_step_time_ms() -> float:
    return Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000.0
    # TIME_PHYSICS_PROCESS retorna segundos no Godot 4
    # multiplicar por 1000 para obter ms
```

> **Confirmar o monitor correto:** `Performance.TIME_PHYSICS_PROCESS` vs `Performance.PHYSICS_3D_ACTIVE_OBJECTS`.
> A lista completa está em `@GlobalScope.Monitor` na documentação.

---

## API de FPS

```gdscript
func get_fps() -> float:
    return Engine.get_frames_per_second()
```

---

## Script de Benchmark — rain_benchmark.gd

> A ser implementado. Estrutura esperada:

```gdscript
# rain_benchmark.gd
# Responsabilidades:
# 1. Receber num_objetos como parâmetro exportado (1000, 5000, 10000)
# 2. Instanciar todas as esferas RigidBody3D com SphereShape3D de raio 0.5
# 3. Durante janela de coleta (ex: 10s), em cada _physics_process:
#    - Ler Performance.TIME_PHYSICS_PROCESS
#    - Ler Engine.get_frames_per_second()
#    - Append em arrays locais
# 4. Ao fim da janela: calcular média dos arrays
# 5. Gravar em CSV via FileAccess.open("user://chuva_godot.csv", FileAccess.WRITE)
# 6. get_tree().reload_current_scene() para próxima run
# 7. Controlar run count via AutoLoad (singleton) persistido entre reloads

@export var num_objetos: int = 1000
@export var total_runs: int = 10
@export var janela_coleta: float = 10.0
@export var output_path: String = "user://chuva_godot.csv"
```

**Singleton de controle de runs (RunManager.gd):**
```gdscript
# AutoLoad: RunManager
# Persiste o número da run atual entre recarregamentos de cena
var run_atual: int = 0
var resultados: Array = []
```

---

## Funil / Área de Contenção

- Construir com `StaticBody3D` + `CollisionShape3D` para cada parede
- Ou usar `CSGBox3D` com `use_collision = true` para prototipagem rápida
- Dimensionar para 10k esferas de raio 0.5m

---

## Observação sobre TIME_PHYSICS_PROCESS

`Performance.TIME_PHYSICS_PROCESS` mede o tempo que o motor levou para executar o passo de física **do frame atual**. No Godot 4 com Jolt multi-threaded, parte desse trabalho pode ocorrer em paralelo — mas o valor reportado ainda representa o tempo de wall-clock da fase de física como percebido pelo game loop.

Comparar com Unity e Unreal considerando essa diferença arquitetural ao interpretar os resultados.

---

## Resultado Esperado

> A preencher após execução.

### Variação 1.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

### Variação 5.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

### Variação 10.000 objetos
| # | Physics Step Time (ms) | FPS | Dentro [M±σ]? |
|---|---|---|---|
| **Média Final** | | | |

---

## Notas

> Registrar aqui observações durante a implementação e execução.
