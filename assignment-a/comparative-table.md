# Tabela Comparativa — Physics Engines (Assignment A)

> Fontes detalhadas em `research-unity.md`, `research-unreal.md`, `research-godot.md`.
> Data de referência: junho 2026.

---

## Motores Subjacentes (Padrão Atual)

| | Unity 6 (LTS) | Unreal Engine 5.8 | Godot 4.6 |
|---|---|---|---|
| **Motor padrão** | NVIDIA PhysX 4.x | Chaos Physics (Epic, proprietário) | Jolt Physics (open source) |
| **Alternativa oficial** | Unity Physics (DOTS/ECS) | — (Chaos é único) | Godot Physics (legado) |
| **Alternativa descontinuada** | Havok Physics for Unity (removido do Pro em 6.3) | PhysX (deprecated em UE 5.1) | — |
| **Solver base** | Impulse-based (PhysX) / XPBD (Unity Physics) | XPBD (Extended Position-Based Dynamics) | Impulse-based (Jolt) |

---

## Tabela Comparativa de Recursos

| Recurso / Estrutura Física | Unity 6 | Unreal Engine 5.8 | Godot 4.6 |
|---|---|---|---|
| **Corpos Rígidos e Colisões** | `Rigidbody` + `Collider` (PhysX); CCD via flag `Collision Detection = Continuous`; também `PhysicsBody` (DOTS/ECS) com CCD. Discrete é o padrão. | Chaos Rigid Body Solver (`FPBDRigidsSolver`); CCD opt-in por componente (`SetUseCCD(true)`) — conservative advancement + especulativo; Discrete é o padrão; dupla precisão (LWC). | `RigidBody3D`; GodotPhysics: CCD via `continuous_cd = true`; Jolt: *speculative contacts* por padrão + *motion clamping*. |
| **Juntas e Restrições** | `HingeJoint`, `FixedJoint`, `SpringJoint`, `CharacterJoint`, `ConfigurableJoint` (6 DoF), `ArticulationBody` (coordenadas reduzidas — solver mais estável para cadeias). ECS: `PhysicsJoint`. | `UPhysicsConstraintComponent`: ball-and-socket, hinge, prismático; motors angulares (SLERP/Twist-Swing); Breakable + Plasticity; `UPhysicalAnimationComponent` (blend física+anim); Constraint Profiles (troca em runtime). | `PinJoint3D`, `HingeJoint3D` (motor + limite ang.), `SliderJoint3D` (translação + rotação 1 eixo), `ConeTwistJoint3D` (ragdoll), `Generic6DOFJoint3D` (6 DoF completo). |
| **Corpos Flexíveis (Soft Bodies / Cloth)** | `Cloth` nativo em `SkinnedMeshRenderer`; colide apenas com `CapsuleCollider` / `SphereCollider` explícitos. **Sem Soft Body volumétrico nativo.** | **Chaos Cloth** (plugin ChaosCloth, CPU, PBD, Skeletal Mesh); **Chaos Flesh** (Experimental, malha tetraedral, soft body volumétrico para deformação muscular — CPU, SIGGRAPH 2024). | `SoftBody3D` nativo (constraints de arestas); comporta-se como tecido — **não volumétrico**. Bugs com Jolt + Area3D; sem physics interpolation. |
| **Destruição Dinâmica (Fracture)** | **Sem sistema nativo.** Depende de Asset Store (DinoFracture, NVIDIA Blast wrapper). Sem equivalente ao Chaos Destruction. | **Chaos Destruction** + **Geometry Collections**: hierarquia de ossos fraturados, pré-autoral no Fracture Mode (Voronoi, Planar, Slice, Brick, Custom) + **simulação real-time**. Field System para controle espacial. Chaos Cache Manager para bake+cinematics. Production Ready desde UE 5.4. | **Sem sistema nativo.** Addons comunitários (Godot-Destruction, Destructibles CSharp). Workaround: substituir mesh por peças pré-quebradas (`RigidBody3D`). |
| **Fluidos e Partículas Físicas** | `Visual Effect Graph` (VFX Graph, GPU) + `Particle System` (CPU). SPH experimental via extensões não-oficiais (FluvioFX). **Sem solver de fluidos nativo interagindo com sólidos.** | **Niagara Fluids** (GPU, solver FLIP PIC/FLIP, colisões via SDF + GDF, Shallow Water 2D, Gas 3D — produção). **Chaos Fluids** (acoplamento bidirecional com rigid bodies — Experimental, sem documentação pública). | `GPUParticles3D` (Vulkan compute, colisão via `GPUParticlesCollision3D`). **Sem solver de fluidos dedicado.** Apenas addons comunitários para fluidos físicos. |
| **Arquitetura Base e Performance** | **Path clássico (PhysX):** worker thread dedicada, stateful. **Path DOTS (Unity Physics):** ECS, C# Job System + Burst Compiler (LLVM), stateless, **determinístico**, Incremental Broadphase, Substepping. Sem GPU physics. "Unified backend" (PhysX 5/MuJoCo) no roadmap para Unity 6.5. | `FPhysScene_Chaos`: Physics Thread dedicado; solver particiona em **islands** paralelos; **Async Physics Tick** (UE 5.4+, timestep fixo); dupla precisão (LWC). Rigid bodies em CPU. GPU apenas em Niagara Fluids. | **GodotPhysics:** single-threaded, main thread. **Jolt (padrão em 4.6):** **multi-threaded nativo** (job system usa todos os cores). Sem GPU physics em nenhum dos dois. `PhysicsServer3D` via GDExtension permite integrar motores externos sem recompilar. |

---

## Resumo: Pontos Fortes e Fracos

| | Unity 6 | Unreal Engine 5.8 | Godot 4.6 |
|---|---|---|---|
| **Melhor em** | Física determinística para multiplayer (DOTS), ArticulationBody para robótica, ecossistema maduro | Destruição dinâmica (Chaos Destruction), fluidos visuais (Niagara Fluids), física esqueletal (ragdolls complexos) | Open source, multi-threading com Jolt, GDExtension extensível, custo zero |
| **Mais fraco em** | Destruição nativa, soft bodies volumétricos, fluidos físicos | Chaos Flesh e Fluids ainda experimentais, Cloth em CPU | Destruição nativa, fluidos físicos, Soft Body com limitações |
| **Arquitetura diferencial** | ECS/DOTS: stateless + Job System + Burst Compiler | Proprietário: LWC dupla precisão + islands paralelos + Async Physics Tick | Modular: swap de motor via Project Settings; GDExtension para motores externos |

---

## Notas para os Slides

- **Tendência 2025–2026:** as três engines convergem para solvers XPBD/PBD, maior multi-threading e integração GPU para efeitos específicos (fluidos, partículas).
- **Ponto a destacar:** Unity é a única com ECS physics genuinamente stateless e determinístico — vantagem clara para jogos online competitivos.
- **Ponto a destacar:** Unreal é a única com destruição procedural AAA nativa (Chaos Destruction) e acoplamento fluido-sólido em desenvolvimento.
- **Ponto a destacar:** Godot é a única open source — customização total do pipeline de física via GDExtension, sem custo de licença.
