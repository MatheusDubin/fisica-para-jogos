# Cenário 1 — A Torre: Godot

> Implementação específica para Godot Engine.
> Base conceitual: `scenario-1-torre-generic.md`
> Versão de referência: Godot 4.6 (Jolt Physics como padrão)

---

## Contexto do Grau A

O Grau A identificou que Godot 4.6 usa **Jolt Physics** como padrão, com solver **impulse-based** e **multi-threading nativo**. O que isso significa para a Torre:

- **Impulse-based como o PhysX, diferente do Chaos:** Jolt e PhysX compartilham o paradigma de solver baseado em impulsos, mas são implementações distintas. A comparação com Unity no cenário de Torre é mais direta do que com Unreal (XPBD).
- **Multi-threading beneficia pouco a Torre:** o job system do Jolt brilha com muitos corpos independentes (Cenário 2). Em uma pilha vertical, as restrições são sequencialmente dependentes (cada cubo depende do de baixo), o que limita o paralelismo útil. O benefício do Jolt aqui é menor que no Cenário 2.
- **Speculative contacts como CCD padrão:** Jolt usa *speculative contacts* por padrão (diferente do `continuous_cd` flag do GodotPhysics). Para a Torre — cenário estático — não tem impacto significativo, mas é parte da arquitetura documentada.
- **GodotPhysics single-threaded** tinha limitações conhecidas em joints/stacks complexos (documentado no Grau A). O Jolt resolveu parte disso. A Torre é uma forma de medir isso empiricamente.
- **Sleep threshold:** documentar os valores de `Sleep Threshold Linear` e `Sleep Threshold Angular` encontrados nas Project Settings antes de rodar — necessário para comparação justa com Unity (0.005 m/s) e Unreal.

---

## Configuração do Projeto

- [ ] Fixed Timestep: `Project Settings > Physics > Common > Physics Ticks Per Second = 50` (equivale a 0.02s)
- [ ] Physics Engine: confirmar `JoltPhysics3D` em `Project Settings > Physics > 3D > Physics Engine`
- [ ] Sleep Threshold: verificar `Project Settings > Physics > 3D > Sleep Threshold Linear / Angular` — documentar valores padrão
- [ ] Gravity: confirmar `Vector3(0, -9.8, 0)` em `Project Settings > Physics > 3D > Default Gravity`

---

## API de Sleep State

Como detectar que um `RigidBody3D` dormiu:

```gdscript
var rb: RigidBody3D = $CuboNode
var dormindo: bool = rb.sleeping
```

Sinal emitido quando o body dorme:
```gdscript
rb.sleeping_state_changed.connect(_on_sleeping_changed)

func _on_sleeping_changed():
    if rb.sleeping:
        print("corpo dormiu")
```

---

## Script de Benchmark — tower_benchmark.gd

> A ser implementado. Estrutura esperada:

```gdscript
# tower_benchmark.gd
# Responsabilidades:
# 1. Instanciar NUM_CUBOS RigidBody3D empilhados verticalmente
# 2. Registrar Time.get_ticks_msec() como t=0 no _physics_process
# 3. A cada _physics_process, checar rb.sleeping para todos os corpos
# 4. Ao detectar sleep total, calcular elapsed e gravar em CSV via FileAccess
# 5. Recarregar cena (get_tree().reload_current_scene()) para próxima run
# 6. Controlar número de runs via AutoLoad singleton ou arquivo de estado

# Parâmetros:
# const NUM_CUBOS = 100
# const TOTAL_RUNS = 10
# const OUTPUT_PATH = "user://torre_godot.csv"
```

**Nota:** usar `_physics_process(delta)` — não `_process(delta)` — para garantir que a checagem ocorre em sincronia com o passo de física.

---

## Profiler / Métricas

Para o Cenário 1, a métrica principal é tempo até sleep — não requer Profiler de física.

Para observar o comportamento em tempo real:
- `Project > Tools > Profiler` (F5 em debug)
- `Performance.get_monitor(Performance.PHYSICS_3D_ACTIVE_OBJECTS)` para contar corpos ativos

---

## Resultado Esperado

> A preencher após execução.

| # | Tempo até Sleep (s) | Jitter? | Colapso? |
|---|---|---|---|
| 1 | | | |
| ... | | | |
| **Média Final** | | | |

---

## Notas

> Registrar aqui observações durante a implementação e execução.
