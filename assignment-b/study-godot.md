# Estudo Genérico — Benchmark em Godot (Assignment B)

> Fase pré-pesquisa: entender a abordagem conceitual antes de ir à documentação.
> Perguntas abertas ao final de cada seção guiam a fase de pesquisa.

---

## 1. Configuração Base

### Fixed Timestep
No Godot, o passo de física é controlado por `physics/common/physics_ticks_per_second` nas Project Settings. Para 50 Hz (0.02s):

```
Project Settings > Physics > Common > Physics Ticks Per Second = 50
```

Ou via código:
```gdscript
Engine.physics_ticks_per_second = 50
```

A física roda no `_physics_process(delta)`, chamado nesse intervalo fixo. `delta` dentro desse callback é sempre `1.0 / physics_ticks_per_second`.

### Escolha do motor: Godot Physics vs Jolt
Como visto no Grau A, Godot 4.6 tem dois motores:
- **Godot Physics:** single-threaded — resultados mais previsíveis, mas gargalo evidente com muitos corpos.
- **Jolt Physics:** multi-threaded — padrão no 4.6, escala melhor com corpos.

**Decisão para o benchmark:** testar com **Jolt Physics** (padrão no 4.6) e registrar o motor nas notas. Se houver tempo, fazer uma variação com Godot Physics para mostrar a diferença — isso seria um dado interessante adicional para o trabalho.

### Primitivas a usar
- `RigidBody3D` + `CollisionShape3D` com `BoxShape3D`
- Criar um cena simples: `RigidBody3D` → `MeshInstance3D` (BoxMesh) + `CollisionShape3D` (BoxShape3D)
- Padronizar massa via `RigidBody3D.mass` e atrito via `PhysicsMaterial`

---

## 2. Cenário 1 — A Torre

### Abordagem de implementação

Instanciar N `RigidBody3D` via `preload` + `instantiate()` em loop:

```gdscript
var box_scene = preload("res://scenes/physics_box.tscn")

func spawn_tower(height: int) -> void:
    for i in range(height):
        var box = box_scene.instantiate()
        box.position = Vector3(0, i * BOX_HEIGHT + OFFSET, 0)
        add_child(box)
```

### Detecção de repouso (sleep state)

O Godot tem a propriedade `RigidBody3D.sleeping` — retorna `true` quando o corpo entrou em sleep.

Para detectar que **a torre inteira** entrou em repouso:

```gdscript
func all_sleeping() -> bool:
    for body in rigid_bodies:
        if not body.sleeping:
            return false
    return true
```

O Godot também tem o sinal `RigidBody3D.sleeping_state_changed` — pode ser mais eficiente que polling para detectar a transição.

O tempo é medido de `t=0` (spawn da última caixa) até `all_sleeping() == true`.

### Perguntas abertas para pesquisa
- [ ] `sleeping_state_changed` está disponível tanto em Godot Physics quanto em Jolt? É confiável?
- [ ] Qual o threshold de sleep padrão no Godot 4.6 / Jolt? Como configurá-lo?
- [ ] `RigidBody3D.sleeping` pode retornar `true` com jitter residual (false positive de sleep)?
- [ ] `instantiate()` + `add_child()` é síncrono no Godot? O corpo já existe na simulação no frame seguinte?
- [ ] Há alguma diferença de comportamento de sleep entre Godot Physics e Jolt que deva ser padronizada?

---

## 3. Cenário 2 — Chuva de Corpos

### Abordagem de implementação

Spawnar N corpos em posições aleatórias dentro de um funil (StaticBody3D com CollisionShape3D formando as paredes):

```gdscript
func spawn_rain(count: int) -> void:
    for i in range(count):
        var box = box_scene.instantiate()
        box.position = Vector3(
            randf_range(-FUNNEL_W, FUNNEL_W),
            randf_range(SPAWN_Y_MIN, SPAWN_Y_MAX),
            randf_range(-FUNNEL_D, FUNNEL_D)
        )
        add_child(box)
```

Para 10.000 corpos, `add_child` em loop pode ser lento. Alternativas no Godot:
- **MultiMesh + RigidBody3D individuais:** renderização batched (MultiMesh) com física individual — verificar viabilidade.
- **`add_child.call_deferred()`:** adia a adição para o fim do frame — pode ajudar a evitar hitch.
- **`PhysicsServer3D.body_create()`:** criar corpos diretamente no servidor de física sem nó de cena — mais rápido, mas sem representação visual automática.

### O que medir

**Physics Step Time:**
```gdscript
# Retorna o tempo da última physics step em segundos (microssegundos na prática)
var physics_time = Performance.get_monitor(Performance.PHYSICS_PROCESS_TIME)
```

**FPS:**
```gdscript
var fps = Engine.get_frames_per_second()
```

**Coleta em `_physics_process()`:**
```gdscript
func _physics_process(_delta: float) -> void:
    if collecting:
        var step_time = Performance.get_monitor(Performance.PHYSICS_PROCESS_TIME)
        samples.append(step_time)
```

### Automação das 10 execuções

O Godot não tem um framework de testes automatizados embutido (há `GUT` como addon). A abordagem mais simples é um `Node` de controle que:

1. Spawna N corpos
2. Aguarda X segundos de simulação
3. Coleta amostras de `PHYSICS_PROCESS_TIME`
4. Destrói todos os filhos (`get_children().map(func(c): c.queue_free())`)
5. Salva resultado em CSV via `FileAccess`
6. Repete por run_index até 10

```gdscript
func save_csv(data: Array, filename: String) -> void:
    var file = FileAccess.open(filename, FileAccess.WRITE)
    for row in data:
        file.store_csv_line(row)
    file.close()
```

### Perguntas abertas para pesquisa
- [ ] `Performance.PHYSICS_PROCESS_TIME` retorna o tempo total do physics tick (incluindo broadphase + narrowphase + solver) ou apenas parte dele?
- [ ] Existe algum monitor de `Performance` mais granular para Jolt específico?
- [ ] `PhysicsServer3D.body_create()` permite simular 10.000 corpos sem nós de cena? Qual o ganho de performance?
- [ ] Como rodar o Godot em modo headless (sem render) para benchmarks limpos? (`--headless` flag existe?)
- [ ] `FileAccess` funciona em builds exportadas (não apenas no editor)?
- [ ] Há alguma forma de desativar renderização mantendo a física ativa para benchmark puro?

---

## 4. Ferramentas de Profiling Disponíveis (genérico)

| Ferramenta | Tipo | Granularidade | Disponível em build? |
|---|---|---|---|
| `Performance.get_monitor()` | API GDScript | Média (por monitor nomeado) | Sim |
| Godot Debugger (Editor) | GUI | Alta (Profiler tab) | Editor only |
| `--profiling` flag (CLI) | CLI | Alta | Development export |
| `OS.get_ticks_usec()` / `OS.get_ticks_msec()` | API GDScript | Manual (wrapping de código) | Sim |
| Jolt internal stats | Interna (a verificar) | Alta | A verificar |

### Monitores relevantes em `Performance` (a confirmar)
- `Performance.PHYSICS_PROCESS_TIME` — tempo da step de física (segundos)
- `Performance.OBJECT_NODE_COUNT` — número de nós na cena
- `Performance.OBJECT_PHYSICS_2D_ACTIVE_OBJECTS` / `_3D_ACTIVE_OBJECTS` — corpos ativos na simulação

---

## 5. Armadilhas Conhecidas

- **GDScript vs C#:** GDScript tem overhead de interpretação. Para spawnar 10.000 objetos, scripts C# (Mono) ou GDNative são significativamente mais rápidos — mas isso afetaria o tempo de spawn, não necessariamente o step time. Definir qual linguagem usar e padronizar.
- **`queue_free()` vs `free()`:** `queue_free()` é assíncrono — o objeto só é deletado no fim do frame. Usar `free()` para limpeza imediata entre runs, ou aguardar um frame após `queue_free()`.
- **Jitter em Godot Physics:** o motor nativo é mais suscetível a jitter em pilhas altas. Isso é um dado válido para o benchmark, mas pode gerar falsos positivos de "não-sleep".
- **`add_child` overhead:** para N > 1.000, o `add_child` em loop pode levar vários frames. Medir o tempo apenas após todos os corpos estarem na cena.
- **Export vs Editor:** o Godot pode ter performance diferente no Editor vs em build exportada. Para resultados válidos, benchmarks devem ser executados em **exports** (headless ou com display).

---

## 6. Perguntas Abertas Gerais (para pesquisa)

- [ ] Qual a forma recomendada pela comunidade Godot para benchmark de física?
- [ ] GDScript vs C# faz diferença significativa no Physics Step Time (a física é C++, mas o script pode gerar overhead de interação)?
- [ ] Jolt multi-threading altera a semântica de `PHYSICS_PROCESS_TIME` (o tempo reportado é wall clock ou soma de threads)?
- [ ] Como configurar `solver_iterations` no Godot/Jolt de forma equivalente ao Unity e Unreal?
- [ ] O `--headless` flag desativa completamente o render ou apenas a janela?
