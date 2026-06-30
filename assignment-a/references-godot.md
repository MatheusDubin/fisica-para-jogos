# Referências — Godot Physics

> Organizadas pelos 6 tópicos da tabela comparativa.
> Data de coleta: junho 2026. Versão de referência: Godot 4.6.

---

## Motor Padrão e Alternativas (Godot Physics vs Jolt)

- Godot Engine. **Godot 4.6 Release Notes — "It's all about your flow"** (janeiro 2026).
  https://godotengine.org/releases/4.6/

- Godot Engine. **Using Jolt Physics** — Godot 4.6 Docs.
  https://docs.godotengine.org/en/4.6/tutorials/physics/using_jolt_physics.html

- GameFromScratch. **Godot 4.4 Gets Native Jolt Physics Support** (março 2025).
  https://gamefromscratch.com/godot-4-4-gets-native-jolt-physics-support/

- StraySpark Studio. **Godot 4.6 Jolt Physics Migration Guide** (2026).
  https://www.strayspark.studio/blog/godot-46-jolt-physics-migration-guide

- Oflight Inc. **Godot 4.5 and 4.6 — Feature Update Recap** (2026).
  https://www.oflight.co.jp/en/columns/godot-4-5-and-4-6-feature-update-2026

- godot-jolt. **Godot Jolt — GitHub Repository** (Jorrit Rouwe / mihe).
  https://github.com/godot-jolt/godot-jolt

- Godot Engine. **GDExtension PhysicsServer3D** — Godot PR #59140.
  https://github.com/godotengine/godot/pull/59140

---

## Corpos Rígidos e Colisões

- Godot Engine. **RigidBody3D class reference** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/classes/class_rigidbody3d.html

- Godot Engine. **Physics introduction** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/tutorials/physics/physics_introduction.html

- Godot Engine. **Collision shapes (3D)** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/tutorials/physics/collision_shapes_3d.html

- godot-jolt. **Known issues and differences vs GodotPhysics** — GitHub Wiki.
  https://github.com/godot-jolt/godot-jolt/wiki/Known-Issues-and-Differences

---

## Juntas e Restrições

- Godot Engine. **Joint3D class reference** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/classes/class_joint3d.html

- Godot Engine. **HingeJoint3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_hingejoint3d.html

- Godot Engine. **SliderJoint3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_sliderjoint3d.html

- Godot Engine. **ConeTwistJoint3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_neatwistjoint3d.html

- Godot Engine. **Generic6DOFJoint3D class reference** — Godot 4.4 Docs.
  https://docs.godotengine.org/en/4.4/classes/class_generic6dofjoint3d.html

---

## Corpos Flexíveis (Soft Bodies / Cloth)

- Godot Engine. **Using SoftBody3D** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/tutorials/physics/soft_body.html

- Godot Engine. **SoftBody3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_softbody3d.html

- godot-jolt. **Support for SoftBody3D in Jolt** — Issue #503.
  https://github.com/godot-jolt/godot-jolt/issues/503

- Godot Engine. **Feature parity: Area3D + SoftBody3D in Jolt** — Issue #111993.
  https://github.com/godotengine/godot/issues/111993

---

## Destruição Dinâmica (Fracture)

> Godot não possui sistema nativo. Referências de addons comunitários abaixo.

- the-dunk. **Godot-Destruction** — GitHub (addon Voronoi runtime, GDScript).
  https://github.com/the-dunk/Godot-Destruction

- Godot Engine. **Add multi-threaded options to 3D physics** — Godot Proposals #483.
  https://github.com/godotengine/godot-proposals/issues/483

---

## Fluidos e Partículas Físicas

- Godot Engine. **GPUParticles3D class reference** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/classes/class_gpuparticles3d.html

- Godot Engine. **GPUParticlesCollision3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_gpuparticlescollision3d.html

- Godot Engine. **CPUParticles3D class reference** — Godot Docs.
  https://docs.godotengine.org/en/stable/classes/class_cpuparticles3d.html

---

## Arquitetura Base e Performance

- Godot Engine. **PhysicsServer3D class reference** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/classes/class_physicsserver3d.html

- Godot Engine. **Physics 3D performance** — Godot Docs (stable).
  https://docs.godotengine.org/en/stable/tutorials/physics/physics_introduction.html#performance

- Godot Engine. **Add multi-threaded options to 3D physics** — Proposals #483.
  https://github.com/godotengine/godot-proposals/issues/483

- godot-jolt. **Godot Jolt — GitHub Repository** (arquitetura e benchmarks internos).
  https://github.com/godot-jolt/godot-jolt

- Rouwe, J. **Jolt Physics** — repositório original.
  https://github.com/jrouwe/JoltPhysics

---

## Fontes Acadêmicas e Técnicas (GDC / SBGames / arXiv)

> Ver também `references-academic.md` para lista completa com resumos.

- Rouwé, J. (Guerrilla Games). **Architecting Jolt Physics for 'Horizon Forbidden West'** — GDC 2022.
  Arquitetura do Jolt: 32 RW mutexes, island building lock-free, redução de stall 500 µs → 10 µs. Motor agora padrão no Godot 4.6.
  https://gdcvault.com/play/1027560/Architecting-Jolt-Physics-for-Horizon
  (PDF): https://jrouwe.nl/architectingjolt/ArchitectingJoltPhysics_Rouwe_Jorrit_Notes.pdf

- Oliveira, S. et al. **Towards Scalable Cloud Gaming Systems: Decoupling Physics from the Game Engine** — SBGames 2023.
  Proposta de offloading de física para VMs cloud; avalia Godot entre as plataformas testadas.
  https://sol.sbc.org.br/index.php/sbgames/article/view/27678

- Anonymous. **Large Viscoelastic Fluid Simulation on GPU** — SBGames 2017.
  Aceleração CUDA de SPH para fluidos viscoelásticos: 7,76× mais rápido; até 1M partículas em real-time. Contextualiza a ausência de solver SPH nativo no Godot.
  https://www.sbgames.org/sbgames2017/papers/ComputacaoFull/175135.pdf

- SBGames 2011. **A Rigid Body Physics Engine for Interactive Applications** — SBGames 2011.
  Contexto histórico brasileiro sobre implementação de engines de física rígida para jogos interativos.
  https://www.sbgames.org/sbgames2011/proceedings/sbgames/papers/comp/short/02-91515_2.pdf

- Kaup, M. et al. **A Review of Nine Physics Engines for Reinforcement Learning Research** — arXiv 2024.
  Compara Jolt/Godot com MuJoCo, PhysX e outros; posiciona open-source vs. proprietário em fidelidade simulação.
  https://arxiv.org/abs/2407.08590
