# Unreal Engine Physics — Relatório de Pesquisa

## Motor Padrão Atual

**Chaos Physics** — motor de física interno desenvolvido pela Epic Games, **padrão desde Unreal Engine 5.0** (abril 2022). Confirmado pelo Director of Physics Engineering Michael Lentine: *"With Unreal Engine 5.0, our internal physics engine Chaos ships as the default physics engine."*

Versão de referência: **UE 5.8** (mais recente documentada).

---

## Histórico de Transição PhysX → Chaos

| Etapa | Versão | Detalhe |
|---|---|---|
| Introdução experimental | UE 4.23 | Chaos disponível via flag de compilação |
| Opt-in em build especial | UE 4.26 | Build `UE_4.26Chaos` com Chaos ativado |
| **Padrão único** | **UE 5.0 (abril 2022)** | Chaos substitui PhysX 3 completamente |
| PhysX depreciado | UE 5.1 | PhysX marcado como deprecated |
| Remoção planejada | Próxima major | Código legado PhysX ainda não totalmente removido |

**Por que migrar?** Large World Coordinates (dupla precisão), física em rede com rollback, destruição dinâmica em real-time, cloth/hair integrados, evolução independente sem licença NVIDIA.

> **Nota:** PhysX ainda existe no código-fonte mas é funcionalmente inacessível sem recompilar a engine (confirmado por engenheiro Epic, junho 2025).

---

## Tabela de Recursos

| Recurso | Sistema / Componente | Detalhes |
|---|---|---|
| **Corpos Rígidos e Colisões** | Chaos Rigid Body Solver (`FPBDRigidsSolver`) | Solver **XPBD (Extended Position-Based Dynamics)**; shapes: convex hulls, esferas, cápsulas, boxes, heightfields, trimeshes; broad phase via BVH; narrow phase via **GJK + EPA**; **CCD opt-in** por componente (`SetUseCCD(true)`) — conservative advancement + especulativo; colisão discreta padrão; **dupla precisão** (LWC). |
| **Juntas e Restrições** | `UPhysicsConstraintComponent` + Physics Asset | Um único componente configurável: ball-and-socket, hinge, prismático; limites lineares e angulares por eixo (Free/Limited/Locked); motors angulares (SLERP ou Twist-and-Swing); **Breakable** (quebra por threshold); **Plasticity** (rest state ajustável); `UPhysicalAnimationComponent` para blend física+animação; Constraint Profiles (troca em runtime). |
| **Ragdoll / Física Esqueletal** | Physics Asset + `USkeletalMeshComponent` | Physics Asset define corpos por osso + constraints; `SetSimulatePhysics(true)` para ragdoll completo; `SetAllBodiesBelowSimulatePhysics(BoneName)` para ragdoll parcial; `Enable Shock Propagation` para estabilidade de cadeias. |
| **Corpos Flexíveis — Tecido** | **Chaos Cloth** (plugin ChaosCloth) | Simulação **CPU-side**, PBD por vértice; apenas em Skeletal Meshes; editor Dataflow (Beta, UE 5.4+); colisores via Physics Asset (cápsulas/esferas/boxes); LOD configurável; **sem GPU path nativo**. |
| **Corpos Flexíveis — Volumétrico** | **Chaos Flesh** (Experimental) | Soft bodies via **malha tetraedral** (TetWild); projetado para **deformação muscular**; gerenciado por Solver Actor; kinematic constraints ligam tet à animação skeletal; colisões com rigid bodies; suporta cache via Sequencer; **CPU-based**, ainda **Experimental** (SIGGRAPH 2024). |
| **Destruição Dinâmica** | **Chaos Destruction** + **Geometry Collections** | Asset dedicado criado via Fracture Mode (Shift+6); hierarquia de ossos fraturados com Connection Graph e Damage Thresholds; ferramentas: Voronoi uniforme/cluster/radial, Planar, Slice, Brick, Mesh; **pré-autoral + simulação real-time**; Field System para controle espacial; Chaos Cache Manager para bake+playback; **Production Ready desde UE 5.4**. |
| **Fluidos — VFX (produção)** | **Niagara Fluids** (plugin, Beta→produção) | Simulação **GPU-based** via Niagara Simulation Stages + Grid Collection; solver **FLIP** (PIC/FLIP híbrido); Shallow Water 2D; Gas/smoke 3D Eulerian; colisões via SDF de Static Meshes, Global Distance Field, heightfield, Physics Asset; renderização via SDF raymarching + Single Layer Water material. |
| **Fluidos — Física Acoplada** | **Chaos Fluids** (Experimental) | Fluidos dentro do solver Chaos com acoplamento bidirecional com rigid bodies; sem documentação oficial pública completa; interação fluido-destruição ainda WiP. |
| **Arquitetura / Performance** | `FPhysScene` → `FPhysScene_Chaos` → `FPBDRigidsSolver` → `FPBDRigidsEvolutionGBF` | Physics Thread (PT) dedicado; troca de dados via lock-free buffers; solver particiona o mundo em **islands** resolvidos em paralelo; **Async Physics Tick** (UE 5.4+, timestep fixo); **GPU: não usado para rigid bodies** — GPU apenas em Niagara Fluids; dupla precisão (LWC) impõe custo mensurável. |

---

## Pontos Fortes

- Motor proprietário: Epic controla o roadmap sem dependências de terceiros.
- **Large World Coordinates** nativo — escalas de quilômetros sem artefatos de floating point.
- **Chaos Destruction** com qualidade cinemática em real-time, Field System para controle artístico e Cache Manager para cinematics.
- Física em rede com rollback integrada nativamente.
- Ecossistema unificado: Cloth, Flesh, Destruction, Fluids e Rigid Bodies no mesmo solver/constraint graph.
- **Async Physics Tick** com timestep fixo — simulações determinísticas.
- Chaos scene queries até **2,6x mais rápidas** que PhysX em benchmarks Epic (UE 5.0).
- Chaos Cloth e Chaos Flesh integrados ao pipeline Dataflow (node-graph).

## Pontos Fracos / Limitações

- Ragdoll e rigid bodies em cenas dinâmicas ainda mais lentos que PhysX em algumas configurações (~54% mais lentos em UE 5.0 — custo parcial do LWC).
- **Chaos Flesh ainda Experimental** — não recomendado para produção.
- **Chaos Fluids sem documentação pública** e sem pipeline oficial.
- **Niagara Fluids sem acoplamento bidirecional** com Chaos rigid bodies — interação fluido-destruição é WiP.
- **Chaos Cloth roda em CPU** — sem GPU path nativo.
- Geometry Collections exigem meshes "water tight" e não-intersectantes — workflow mais complexo.
- PhysX não removido completamente do código-fonte (código legado residual).

---

## Referências

- [Chaos Scene Queries and Rigid Body Engine in UE5 — Epic Tech Blog (maio 2022)](https://www.unrealengine.com/en-US/tech-blog/chaos-scene-queries-and-rigid-body-engine-in-ue5)
- [Chaos Flesh Overview — UE 5.8 Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-flesh-overview)
- [Chaos Destruction in Unreal Engine — UE 5.8 Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-in-unreal-engine)
- [Geometry Collections User Guide — UE 5.7 Docs](https://dev.epicgames.com/documentation/unreal-engine/geometry-collections-user-guide)
- [Niagara Fluids — UE 5.7 Docs](https://dev.epicgames.com/documentation/en-us/unreal-engine/niagara-fluids-in-unreal-engine)
- [Physics Constraint Reference — UE 5.8 Docs](https://dev.epicgames.com/documentation/unreal-engine/physics-constraint-reference-in-unreal-engine)
- [Future of Chaos, PhysX and other physics engines — Epic Forums (junho 2025)](https://forums.unrealengine.com/t/future-of-chaos-physx-and-other-physics-engines/2610544)
- [Exploring Chaos: Flesh, Fluids, and Fractures — Unreal Fest 2023](https://dev.epicgames.com/community/learning/talks-and-demos/Eopj/unreal-engine-exploring-chaos-flesh-fluids-and-fractures)
