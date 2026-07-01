# Referências — Unity Physics

> Organizadas pelos 6 tópicos da tabela comparativa.
> Data de coleta: junho 2026. Versão de referência: Unity 6 / Unity 6.4 (6000.4).

---

## Motor Padrão e Alternativas

- Unity Technologies. **Built-in 3D Physics overview** — Unity Manual 6.4.
  https://docs.unity3d.com/6000.4/Documentation/Manual/PhysicsOverview.html

- Unity Technologies. **Unity Physics package** (`com.unity.physics`) — Changelog 1.3.x.
  https://docs.unity3d.com/Packages/com.unity.physics@1.3/changelog/CHANGELOG.html

- Unity Technologies. **ECS (Data-Oriented Technology Stack)** — unity.com/ecs.
  https://unity.com/ecs

- Unity Technologies. **Physics development status and next milestones** (maio 2025).
  https://discussions.unity.com/t/physics-development-status-and-next-milestones-may-2025/1637988

- Unity Technologies. **Unity ends Havok Physics support — no longer included with Unity Pro** (novembro 2025).
  https://discussions.unity.com/t/unity-ends-havok-physics-support-no-longer-included-with-unity-pro/1694786

- Microsoft / Havok. **Havok Physics for Unity is now production-ready** — Microsoft Developer Blog (2023).
  https://developer.microsoft.com/en-us/games/articles/2023/01/havok-physics-for-unity-is-now-production-ready/

---

## Corpos Rígidos e Colisões

- Unity Technologies. **Rigidbody component reference** — Unity Manual 6.4.
  https://docs.unity3d.com/6000.4/Documentation/Manual/class-Rigidbody.html

- Unity Technologies. **Continuous Collision Detection (CCD)** — Unity Manual.
  https://docs.unity3d.com/Manual/ContinuousCollisionDetection.html

- Unity Technologies. **Collider component overview** — Unity Manual.
  https://docs.unity3d.com/Manual/CollidersOverview.html

---

## Juntas e Restrições

- Unity Technologies. **ConfigurableJoint** — Unity Manual (Unity 6).
  https://docs.unity3d.com/6000.0/Documentation/Manual/class-ConfigurableJoint.html

- Unity Technologies. **ArticulationBody** — Unity Manual.
  https://docs.unity3d.com/Manual/class-ArticulationBody.html

- Unity Technologies. **HingeJoint** — Unity Manual.
  https://docs.unity3d.com/Manual/class-HingeJoint.html

- Unity Technologies. **PhysicsJoint** (DOTS) — Unity Physics package docs.
  https://docs.unity3d.com/Packages/com.unity.physics@latest/manual/joints.html

---

## Corpos Flexíveis (Soft Bodies / Cloth)

- Unity Technologies. **Cloth component** — Unity Manual.
  https://docs.unity3d.com/Manual/class-Cloth.html

- Unity Technologies. **Skinned Mesh Renderer** — Unity Manual.
  https://docs.unity3d.com/Manual/class-SkinnedMeshRenderer.html

---

## Destruição Dinâmica (Fracture)

> Unity não possui sistema nativo. Nenhuma referência oficial aplicável.
> Referências de terceiros abaixo.

- Dinos Kousidis. **DinoFracture — Fracture library for Unity** — GitHub.
  https://github.com/dkusidis/DinoFracture *(addon comunitário)*

- NVIDIA. **Blast SDK** — PhysX Blast destruction library.
  https://developer.nvidia.com/blast *(requer integração manual)*

---

## Fluidos e Partículas Físicas

- Unity Technologies. **Visual Effect Graph (VFX Graph)** — Unity Manual.
  https://docs.unity3d.com/Packages/com.unity.visualeffectgraph@latest

- Unity Technologies. **Particle System overview** — Unity Manual.
  https://docs.unity3d.com/Manual/PartSysReference.html

- Lehmuskallio, A. et al. **SPH-based fluid simulation with DOTS/Burst Compiler in Unity** (2025) — pesquisa acadêmica.
  *(disponível via Google Scholar: "SPH Unity DOTS Burst Compiler 2025")*

---

## Arquitetura Base e Performance

- Unity Technologies. **Data-Oriented Technology Stack (DOTS)** — documentação oficial.
  https://docs.unity3d.com/Packages/com.unity.entities@latest

- Unity Technologies. **Burst Compiler** — documentação oficial.
  https://docs.unity3d.com/Packages/com.unity.burst@latest

- Unity Technologies. **C# Job System** — Unity Manual.
  https://docs.unity3d.com/Manual/JobSystem.html

- Unity Technologies. **Physics programming overview** — unity.com/solutions.
  https://unity.com/solutions/programming-physics

- Make, J. **Integrating NVIDIA PhysX 5.6 with Unity Engine** — Medium/SpicyTech.
  https://jmake.medium.com/integrating-nvidia-physx-5-6-with-unity-engine-c87215628c60

---

## Fontes Acadêmicas e Técnicas (GDC / SIGGRAPH / arXiv)

> Ver também `references-academic.md` para lista completa com resumos.

- Balakshin, A. **ECS in Practice: The Case of Unity** — GDC 2024.
  Implementação prática de DOTS/física stateless em produção; impacto do Burst Compiler em performance.
  https://gdcvault.com/play/1034295/ECS-in-Practice-The-Case

- Macklin, M. et al. **Small Steps in Physics Simulation** — SIGGRAPH/SCA 2019.
  Base matemática para substeps em solvers; fundação do Unity Physics (stateless, determinístico).
  https://mmacklin.com/smallsteps.pdf

- Chentanez, N. et al. (NVIDIA). **Game Physics on the GPU** — GDC 2018.
  Migração de PGS/TGS e broadphase para GPU no PhysX — backend padrão do Unity.
  https://gdcvault.com/play/1024345/Game-Physics-on-the-GPU

- NVIDIA. **PhysX 4: Raising the Fidelity and Performance of Physics Simulation** — GTC 2019.
  Introdução do solver TGS na **biblioteca** PhysX 4. **Nota (corrigido no Grau B):** TGS é *opção* da biblioteca; o Unity clássico (GameObject) usa **PGS por padrão** (`DynamicsManager.asset → m_SolverType: 0`) — TGS é opt-in, não o default.
  https://developer.download.nvidia.com/video/gputechconf/gtc/2019/presentation/s9990-physx-4-raising-the-fidelity-and-performance-of-physics-simulation-in-games.pdf

- Kaup, M. et al. **A Review of Nine Physics Engines for Reinforcement Learning Research** — arXiv 2024.
  Compara Unity com MuJoCo, PhysX e outros para RL; destaca ArticulationBody como diferencial.
  https://arxiv.org/abs/2407.08590
