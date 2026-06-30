# Godot Physics — Relatório de Pesquisa

## Motor Padrão Atual

**Godot 4.x** possui dois motores de física 3D:

- **Godot Physics** — motor nativo escrito do zero pela equipe Godot. Presente desde Godot 4.0.
- **Jolt Physics** — integrado como módulo oficial no **Godot 4.4** (março 2025). A partir do **Godot 4.6** (janeiro 2026), Jolt passou a ser o **padrão para novos projetos 3D**, com o label "experimental" removido.

O Jolt foi criado por **Jorrit Rouwe** e usado em produções AAA (*Horizon Forbidden West*, *Death Stranding 2*). No Godot 4.4 foi integrado como módulo de engine — não addon externo. A versão standalone como GDExtension (`godot-jolt`) ainda existe na Asset Library para versões anteriores (4.0–4.3).

Para trocar o motor: `Project Settings > Physics > 3D > Physics Engine` → `GodotPhysics3D` ou `JoltPhysics3D`.

---

## Godot Physics vs Jolt Physics

| Critério | Godot Physics | Jolt Physics |
|---|---|---|
| Disponível desde | Godot 4.0 | Godot 4.4 (addon antes) |
| Padrão em novos projetos | Godot 4.0–4.5 | **Godot 4.6+** |
| Threading | Single-threaded (main thread) | **Multi-threaded nativo** |
| Performance (muitos corpos) | Degradação marcada acima de ~500 bodies | Dramaticamente mais rápido |
| Performance (poucos corpos) | Equivalente | Equivalente |
| Estabilidade do solver | Limitações em joints complexas | Estável e determinístico |
| CCD | Via flag `continuous_cd` em `RigidBody3D` | Speculative contacts + motion clamping (mais confiável) |
| SoftBody3D | Suportado estável | Suportado (experimental, com bugs) |
| WorldBoundaryShape3D | Sim | **Não suportado** |
| HeightMapShape3D (grandes) | Mais rápido em terrenos grandes | Mais lento em alguns casos |

---

## Tabela de Recursos

| Recurso | Sistema / Componente | Detalhes |
|---|---|---|
| **Corpos Rígidos e Colisões** | `RigidBody3D`, `StaticBody3D`, `CharacterBody3D`, `AnimatableBody3D` | CCD: `continuous_cd = true` em `RigidBody3D` (GodotPhysics); Jolt usa *speculative contacts* por padrão + *motion clamping*. Shapes: `BoxShape3D`, `SphereShape3D`, `CapsuleShape3D`, `CylinderShape3D`, `ConvexPolygonShape3D`, `ConcavePolygonShape3D`, `HeightMapShape3D`, `WorldBoundaryShape3D` (não suportada em Jolt). |
| **Juntas e Restrições** | `Joint3D` (base), `PinJoint3D`, `HingeJoint3D`, `SliderJoint3D`, `ConeTwistJoint3D`, `Generic6DOFJoint3D` | 5 tipos de joints 3D. `HingeJoint3D`: rotação com limite angular e motor. `SliderJoint3D`: translação + rotação em 1 eixo. `ConeTwistJoint3D`: articulações ragdoll (ombros, quadris). `Generic6DOFJoint3D`: controle individual de 6 graus de liberdade. |
| **Corpos Flexíveis / Cloth** | `SoftBody3D` | Simulação por malha de vértices (constraints de arestas) — **não é volumétrico**, comporta-se como tecido. Usa `MeshInstance3D` para deformação visual. Limitações: sem physics interpolation; sem interação com `Area3D` no Jolt; posição distante da origem causa bugs no Jolt. |
| **Destruição Dinâmica** | **Sem sistema nativo** | Não há fractura procedimental nativa. Workarounds: substituir mesh por peças pré-quebradas (`RigidBody3D`) no impacto. Addons: **Godot-Destruction** (Voronoi, runtime), **Destructibles CSharp**, **Jummit's Destruction Plugin** — todos comunitários, sem qualidade AAA. |
| **Fluidos e Partículas** | `GPUParticles3D` + `CPUParticles3D` + `ParticleProcessMaterial` | Sem solver de fluidos dedicado. `GPUParticles3D` roda em Vulkan compute shaders; colisão com `GPUParticlesCollision3D` (SDF ou heightmap). Para fluidos físicos: apenas addons (ex: *Fluid Simulation* GPU, 2025) ou `PhysicsServer2D` para 2D. Sem SPH, PBF ou FLIP solver embutido. |
| **Arquitetura / Performance** | `PhysicsServer3D` (GodotPhysics: single-thread; Jolt: multi-thread) + GDExtension | **GodotPhysics:** single-threaded, sem GPU. **JoltPhysics:** multi-threaded nativo (job system do Jolt, usa todos os cores), sem GPU para física. GDExtension permite registrar `PhysicsServer3D` em C++ (permite integrar PhysX, Bullet, etc. sem recompilar). Sem aceleração GPU para física rígida em nenhum dos dois. |

---

## Pontos Fortes

- **Jolt como padrão em 4.6:** novos projetos nascem com o motor mais performático sem configuração extra.
- **Multi-threading no Jolt:** escala com múltiplos núcleos para simulações densas.
- **API unificada:** trocar de motor não requer mudança de código — `PhysicsServer3D` e nós são os mesmos.
- **5 tipos de joints nativos:** cobertura completa para ragdolls, portas, veículos.
- **SoftBody3D nativo:** suporte integrado sem addon.
- **GPUParticles3D:** partículas com colisão via GPU (SDFs).
- **GDExtension:** permite integrar PhysX, Bullet ou qualquer solver externo sem recompilar a engine.
- **Open source:** toda a stack de física é auditável e modificável.

## Pontos Fracos / Limitações

- **Sem destruição nativa:** fractura procedimental depende de addons comunitários; nenhum tem qualidade AAA.
- **Sem solver de fluidos:** fluidos são visuais (GPU particles), não físicos — sem equivalente ao Niagara Fluids.
- **SoftBody3D com limitações sérias:** sem interpolação de física, bugs com Jolt + Area3D, degradação de performance com malhas densas.
- **GodotPhysics single-threaded:** projetos legados (pré-4.6) sofrem em cenas de física densa.
- **WorldBoundaryShape3D não suportada em Jolt** (plano infinito) — exige workaround.
- **HeightMapShape3D lenta em Jolt** para terrenos muito grandes.
- **Sem GPU physics:** todo processamento rígido e soft é CPU.
- **Jolt com feature gaps:** interação Area3D ↔ SoftBody3D ausente; alguns comportamentos diferem entre GodotPhysics e Jolt.

---

## Referências

- [Godot 4.4 Gets Native Jolt Physics Support — GameFromScratch](https://gamefromscratch.com/godot-4-4-gets-native-jolt-physics-support/)
- [Godot 4.6 Release Notes](https://godotengine.org/releases/4.6/)
- [Using Jolt Physics — Godot 4.6 Docs](https://docs.godotengine.org/en/4.6/tutorials/physics/using_jolt_physics.html)
- [GitHub: godot-jolt/godot-jolt](https://github.com/godot-jolt/godot-jolt)
- [Using SoftBody3D — Godot Docs](https://docs.godotengine.org/en/stable/tutorials/physics/soft_body.html)
- [Joint3D Class Reference — Godot Docs](https://docs.godotengine.org/en/stable/classes/class_joint3d.html)
- [GPUParticles3D — Godot Docs](https://docs.godotengine.org/en/stable/classes/class_gpuparticles3d.html)
- [GitHub: Godot-Destruction (the-dunk)](https://github.com/the-dunk/Godot-Destruction)
- [Support for SoftBody3D in Jolt — Issue #503](https://github.com/godot-jolt/godot-jolt/issues/503)
- [Godot 4.6 Jolt Migration Guide — StraySpark](https://www.strayspark.studio/blog/godot-46-jolt-physics-migration-guide)
