# How-To: Godot — Setup, Profiling e Métricas

> Guia prático para quem está saindo do zero ou está enferrujado.
> Versão de referência: Godot 4.6 (Jolt Physics como padrão).
> Linguagem: GDScript.

---

## 1. Criar o Projeto

1. Abrir **Godot 4.6**
2. Clicar em **New Project**
3. Configurações:
   - Project Name: `project-f-godot`
   - Renderer: **Forward+** (padrão para 3D)
   - Version Control: None
4. **Create & Edit**

---

## 2. Configurar Fixed Timestep

`Project > Project Settings > Physics > Common`

- **Physics Ticks Per Second:** `50` (equivale a 0.02s por tick)

Confirmar: `1 / 50 = 0.02s` ✓

---

## 3. Confirmar Motor de Física (Jolt)

`Project > Project Settings > Physics > 3D`

- **Physics Engine:** deve estar em `JoltPhysics3D`
- Se estiver em `GodotPhysics3D`, trocar para `JoltPhysics3D` e reiniciar o editor

---

## 4. Configurar Gravity e Sleep

Na mesma tela (`Project Settings > Physics > 3D`):

**Anotar e não alterar:**

| Configuração | Valor padrão (anotar aqui) |
|---|---|
| Default Gravity | ___ |
| Default Gravity Vector | ___ |
| Sleep Linear Velocity | ___ |
| Sleep Angular Velocity | ___ |

Gravity deve ser `9.8` (positivo — Godot inverte internamente; ou `-9.8` dependendo da versão).

---

## 5. Desativar V-Sync

`Project > Project Settings > Display > Window`

- **V-Sync Mode:** `Disabled`

---

## 6. Medir Physics Step Time via Código

Godot expõe métricas de performance via `Performance` singleton. Não precisa de Profiler especial — é acessível direto em GDScript.

```gdscript
func _physics_process(delta: float) -> void:
    # TIME_PHYSICS_PROCESS retorna segundos — converter para ms
    var step_time_ms: float = Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000.0
    print("Physics Step Time: %.3f ms" % step_time_ms)
```

> **Por que `_physics_process` e não `_process`?**
> `_physics_process` roda em sincronia com o passo de física (50 vezes por segundo com nosso timestep). Ler a métrica aqui garante que estamos lendo o valor do passo que acabou de ocorrer.

### Outros monitores úteis

```gdscript
# Número de corpos rígidos ativos (não dormindo)
var ativos = Performance.get_monitor(Performance.PHYSICS_3D_ACTIVE_OBJECTS)

# Número de colisões detectadas neste frame
var colisoes = Performance.get_monitor(Performance.PHYSICS_3D_COLLISION_PAIRS)

# FPS atual
var fps = Engine.get_frames_per_second()
```

Lista completa: `@GlobalScope.Monitor` na documentação de Godot.

---

## 7. Medir FPS

```gdscript
func _process(delta: float) -> void:
    var fps: float = Engine.get_frames_per_second()
```

> Usar `_process` para FPS (renderização) e `_physics_process` para métricas de física — são callbacks diferentes.

---

## 8. Detectar Sleep State (Cenário 1 — Torre)

```gdscript
# Propriedade direta:
var rb: RigidBody3D = $NomeDoCubo
var esta_dormindo: bool = rb.sleeping

# Sinal emitido quando o estado muda:
rb.sleeping_state_changed.connect(_on_cubo_dormiu.bind(rb))

func _on_cubo_dormiu(corpo: RigidBody3D) -> void:
    if corpo.sleeping:
        print("corpo dormiu")
```

### Script completo de detecção para a Torre

```gdscript
extends Node3D

var corpos: Array[RigidBody3D] = []
var tempo_inicio: float = 0.0
var medindo: bool = false

func _ready() -> void:
    # spawnar cubos aqui (ver seção 10)
    tempo_inicio = Time.get_ticks_msec() / 1000.0
    medindo = true

func _physics_process(delta: float) -> void:
    if not medindo:
        return

    var dormindo: int = 0
    for corpo in corpos:
        if corpo.sleeping:
            dormindo += 1

    if dormindo == corpos.size():
        var tempo_ate_sleep: float = (Time.get_ticks_msec() / 1000.0) - tempo_inicio
        print("Torre dormiu em: %.2f segundos" % tempo_ate_sleep)
        medindo = false
        _salvar_resultado(tempo_ate_sleep)
        _proxima_run()
```

> **`Time.get_ticks_msec()`** retorna milissegundos desde o início do programa — dividir por 1000.0 para obter segundos. Mais preciso que `Time.get_time()` para medir intervalos.

---

## 9. Salvar Dados em CSV

```gdscript
func _salvar_csv(caminho: String, linha: String) -> void:
    var arquivo = FileAccess.open(caminho, FileAccess.READ_WRITE)
    if arquivo == null:
        # arquivo não existe ainda — criar com header
        arquivo = FileAccess.open(caminho, FileAccess.WRITE)
        arquivo.store_line("run,variacao,physics_step_ms,fps")

    arquivo.seek_end()  # ir para o final do arquivo
    arquivo.store_line(linha)
    arquivo.close()

# Uso:
# _salvar_csv("user://torre_godot.csv", "%d,%.3f" % [run_atual, tempo_ate_sleep])
# _salvar_csv("user://chuva_godot.csv", "%d,%d,%.3f,%.1f" % [run_atual, variacao, step_ms, fps])
```

O caminho `user://` é o diretório de dados do usuário da aplicação:
- Windows: `%APPDATA%/Godot/app_userdata/project-f-godot/`
- macOS: `~/Library/Application Support/Godot/app_userdata/project-f-godot/`

Para abrir a pasta: `Project > Open User Data Folder` no editor.

---

## 10. Automatizar 10 Runs (Recarregar Cena)

O problema: recarregar a cena destrói tudo — inclusive o contador de runs. A solução é um **AutoLoad (singleton)** que persiste entre recarregamentos.

### Criar o Singleton de Runs

1. Criar arquivo `run_manager.gd`:

```gdscript
# run_manager.gd
extends Node

var run_atual: int = 0
var total_runs: int = 10
var resultados: Array = []

func proxima_run(cena_atual: String) -> void:
    run_atual += 1
    if run_atual >= total_runs:
        print("Benchmark concluído! %d runs completadas." % total_runs)
        _processar_resultados()
        return
    get_tree().reload_current_scene()

func registrar(valor: float) -> void:
    resultados.append(valor)

func _processar_resultados() -> void:
    # chamar aqui a função de cálculo estatístico
    pass
```

2. Registrar como AutoLoad:
   - `Project > Project Settings > AutoLoad`
   - Clicar em **Add** (ícone de pasta)
   - Selecionar `run_manager.gd`
   - Name: `RunManager`
   - **Enable** marcado
   - OK

3. Acessar em qualquer script:

```gdscript
RunManager.proxima_run(scene_file_path)
RunManager.registrar(tempo_ate_sleep)
```

---

## 11. Criar os Objetos Físicos

### Cubo (Cenário 1 — Torre)

```gdscript
func _criar_cubo(posicao: Vector3) -> RigidBody3D:
    var rb = RigidBody3D.new()
    rb.mass = 1.0
    rb.gravity_scale = 1.0

    var mesh_instance = MeshInstance3D.new()
    var box_mesh = BoxMesh.new()
    box_mesh.size = Vector3(1, 1, 1)  # 1m × 1m × 1m
    mesh_instance.mesh = box_mesh
    rb.add_child(mesh_instance)

    var col = CollisionShape3D.new()
    var box_shape = BoxShape3D.new()
    box_shape.size = Vector3(1, 1, 1)
    col.shape = box_shape
    rb.add_child(col)

    # PhysicsMaterial para atrito e restituição
    var mat = PhysicsMaterial.new()
    mat.friction = 0.5
    mat.rough = false
    mat.bounce = 0.0  # ← crítico para Torre
    rb.physics_material_override = mat

    rb.position = posicao
    add_child(rb)
    return rb
```

### Esfera (Cenário 2 — Chuva)

```gdscript
func _criar_esfera(posicao: Vector3) -> RigidBody3D:
    var rb = RigidBody3D.new()
    rb.mass = 1.0

    var mesh_instance = MeshInstance3D.new()
    mesh_instance.mesh = SphereMesh.new()  # raio padrão = 0.5m ✓
    rb.add_child(mesh_instance)

    var col = CollisionShape3D.new()
    var sphere_shape = SphereShape3D.new()
    sphere_shape.radius = 0.5
    col.shape = sphere_shape
    rb.add_child(col)

    var mat = PhysicsMaterial.new()
    mat.friction = 0.4
    mat.bounce = 0.3  # um pouco de bounce para a Chuva
    rb.physics_material_override = mat

    rb.position = posicao
    add_child(rb)
    return rb
```

### Chão e Funil (Static)

```gdscript
func _criar_parede_estatica(posicao: Vector3, tamanho: Vector3) -> StaticBody3D:
    var sb = StaticBody3D.new()

    var col = CollisionShape3D.new()
    var box_shape = BoxShape3D.new()
    box_shape.size = tamanho
    col.shape = box_shape
    sb.add_child(col)

    sb.position = posicao
    add_child(sb)
    return sb
```

---

## 12. Usar o Profiler do Editor (visualização)

Durante o desenvolvimento (não para coleta de dados finais):

1. `Project > Tools > Profiler` ou pressionar **Debugger** na barra inferior
2. Aba **Profiler**
3. Clicar em **Start** antes de dar play
4. Dar play na cena
5. Observar as barras:
   - **Physics Process:** tempo do passo de física — é o que medimos via `TIME_PHYSICS_PROCESS`
   - **Physics:** colisões e broadphase
6. Clicar em qualquer frame para ver o breakdown detalhado

> O Profiler do editor **não deve ser usado para os dados finais** — adiciona overhead. Usar apenas para desenvolvimento e debugging visual. Os dados finais vêm do `Performance.get_monitor()` em builds exportadas.

---

## 13. Exportar Build para Coleta Final

Os dados devem ser coletados em **build exportada**, não no editor (o editor adiciona overhead de debugging).

1. `Project > Export`
2. Adicionar preset: **Windows Desktop** (ou macOS/Linux conforme sua plataforma)
3. Export Path: escolher pasta
4. **Export Project** (não "Export PCK/Zip")
5. Rodar o executável exportado para coletar os dados finais

---

## 14. Checklist antes de Rodar

- [ ] Physics Ticks Per Second = 50 confirmado
- [ ] Physics Engine = JoltPhysics3D confirmado
- [ ] V-Sync desativado
- [ ] Gravity e sleep thresholds anotados (não alterados)
- [ ] `PhysicsMaterial.bounce = 0.0` nos cubos (Torre)
- [ ] AutoLoad `RunManager` registrado e funcionando
- [ ] `_physics_process` usado para métricas (não `_process`)
- [ ] CSV sendo gravado com `FileAccess` em `user://`
- [ ] Build exportada (não rodar no editor para dados finais)

---

## 15. Referência Rápida de APIs

| O que fazer | API |
|---|---|
| Physics Step Time | `Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000.0` |
| Corpos ativos | `Performance.get_monitor(Performance.PHYSICS_3D_ACTIVE_OBJECTS)` |
| FPS | `Engine.get_frames_per_second()` |
| Corpo está dormindo? | `rigid_body.sleeping` |
| Sinal de sleep | `rigid_body.sleeping_state_changed` |
| Tempo atual (ms) | `Time.get_ticks_msec()` |
| Recarregar cena | `get_tree().reload_current_scene()` |
| Salvar arquivo | `FileAccess.open("user://arquivo.csv", FileAccess.WRITE)` |
| Abrir pasta user:// | `Project > Open User Data Folder` no editor |
