# Unity Physics — Relatório de Pesquisa

## Motor Padrão Atual

**NVIDIA PhysX** (versão 4.x integrada pela Unity — não é PhysX 5, que está listado como backend futuro via terceiros).
Confirmado como padrão em Unity 6.4 (6000.4): *"Unity's default built-in 3D physics system is an integration of the Nvidia PhysX engine."*
Aplica-se a projetos orientados a GameObjects (o modelo tradicional de desenvolvimento Unity).

---

## Alternativas Oficiais

| Pacote | Contexto | Status (junho 2026) |
|---|---|---|
| `com.unity.physics` (Unity Physics) | ECS/DOTS apenas | Produção — versão 1.3.x / 6.5.x |
| `com.havok.physics` (Havok Physics for Unity) | ECS/DOTS apenas | **Descontinuado** como benefício Unity Pro em Unity 6.3+; licença direta via Microsoft Havok (~$50k/produto) |
| PhysX 5 / MuJoCo | Backend customizado futuro | Em roadmap para Unity 6.5+ — ainda não disponível |

> **Nota crítica:** A Unity anunciou em novembro de 2025 que, a partir do Unity 6.3 LTS, o Havok Physics **não é mais incluído** nas assinaturas Pro/Enterprise/Industry. Suporte mantido apenas para Unity 2022 LTS e Unity 6.0 LTS.

---

## Tabela de Recursos

| Recurso | Sistema/Componente | Detalhes |
|---|---|---|
| **Corpos Rígidos e Colisões** | `Rigidbody` + `Collider` (PhysX) / `PhysicsBody` + `PhysicsCollider` (ECS) | CCD habilitado por flag no `Rigidbody` (`Collision Detection = Continuous`). Discreto é o padrão. Shapes: Box, Sphere, Capsule, Mesh, Terrain, Wheel. |
| **Juntas e Restrições** | `HingeJoint`, `FixedJoint`, `SpringJoint`, `CharacterJoint`, `ConfigurableJoint`, `ArticulationBody` | `ConfigurableJoint`: controla translação e rotação em 6 DoF. `ArticulationBody` (Unity 2020.1+): solver de redução de coordenadas — ideal para robótica e ragdolls estáveis. ECS usa `PhysicsJoint` customizável. |
| **Corpos Flexíveis / Cloth** | `Cloth` (componente nativo) | Funciona com `SkinnedMeshRenderer`. **Sem Soft Body volumétrico nativo.** Cloth colide apenas com `CapsuleCollider` e `SphereCollider` explicitamente configurados — não com geometria arbitrária. Unity decidiu em 2024 **não deprecar** e planeja melhorias. |
| **Destruição Dinâmica** | **Sem sistema nativo** | Não existe fractura procedural embutida. Opções: pré-fractura com DinoFracture ou NVIDIA Blast wrapper; soluções de Asset Store. Sem equivalente ao Chaos Destruction (Unreal). |
| **Fluidos e Partículas Físicas** | `Particle System` (CPU) / `Visual Effect Graph — VFX Graph` (GPU) | Sem simulação de fluido nativo interagindo com sólidos. VFX Graph roda na GPU e suporta SPH experimental via extensões (ex: FluvioFX), mas não é pacote oficial. |
| **Arquitetura / Performance** | **PhysX path:** multi-thread parcial. **DOTS path:** `com.unity.physics` + `Burst Compiler` + `C# Job System` | PhysX (GameObject): stateful, determinístico, worker thread dedicada. Unity Physics (ECS): **stateless**, **determinístico**, Job System para paralelismo total em CPU, compilado com Burst. **Sem aceleração por GPU** no pipeline de física. Roadmap: "Unified physics backend" previsto para Unity 6.5. |

---

## Pontos Fortes

- **PhysX maduro:** 20+ anos de uso em jogos comerciais; integração profunda com GameObjects.
- **ArticulationBody:** solver de coordenadas reduzidas — diferencial real vs. engines concorrentes para robótica/ragdolls estáveis.
- **Unity Physics (DOTS):** totalmente stateless e determinístico — excelente para multiplayer com rollback netcode. Burst Compiler entrega ganhos de 20–100x em cenas com milhares de corpos.
- **Incremental Broadphase** (Unity Physics 1.3+, Unity 6): atualização incremental da BVH — redução de custo em cenas com muitos corpos quase estáticos.
- **Substepping** (Unity Physics 1.3+): melhora estabilidade do solver para constraint chains.

## Pontos Fracos / Limitações

- **Sem Soft Bodies volumétricos nativos:** sem equivalente ao NVIDIA FleX ou Chaos Flesh.
- **Sem destruição procedural nativa:** requer Asset Store ou soluções customizadas.
- **Sem simulação de fluidos nativa.**
- **Dois mundos paralelos sem interoperabilidade:** GameObjects (PhysX) e Entities/ECS (Unity Physics) não interagem na mesma simulação — limitação arquitetural fundamental.
- **Havok descontinuado como benefício:** removido do Unity Pro a partir de 6.3.
- **PhysX versão desatualizada:** ainda em 4.x; PhysX 5 (fluidos, soft bodies, GPU) só como backend externo futuro.

---

## Referências

- [Unity Manual: Built-in 3D Physics (Unity 6.4)](https://docs.unity3d.com/6000.4/Documentation/Manual/PhysicsOverview.html)
- [Unity Physics package changelog 1.3.x](https://docs.unity3d.com/Packages/com.unity.physics@1.3/changelog/CHANGELOG.html)
- [Physics development status and next milestones — May 2025 (oficial Unity)](https://discussions.unity.com/t/physics-development-status-and-next-milestones-may-2025/1637988)
- [Unity ends Havok Physics support — November 2025](https://discussions.unity.com/t/unity-ends-havok-physics-support-no-longer-included-with-unity-pro/1694786)
- [Unity Manual: ArticulationBody](https://docs.unity3d.com/Manual//class-ArticulationBody.html)
- [Unity Manual: Cloth](https://docs.unity3d.com/Manual/class-Cloth.html)
- [Unity ECS](https://unity.com/ecs)
