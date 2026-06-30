# Referências — Unreal Engine Physics

> Organizadas pelos 6 tópicos da tabela comparativa.
> Data de coleta: junho 2026. Versão de referência: Unreal Engine 5.8.

---

## Motor Padrão e Histórico de Transição

- Lentine, M. (Epic Games). **Chaos Scene Queries and Rigid Body Engine in UE5** — Epic Tech Blog (maio 2022).
  https://www.unrealengine.com/en-US/tech-blog/chaos-scene-queries-and-rigid-body-engine-in-ue5

- Epic Games. **Physics in Unreal Engine** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/unreal-engine/physics-in-unreal-engine

- Epic Games Community. **Future of Chaos, PhysX and other physics engines** — Epic Forums (junho 2025).
  https://forums.unrealengine.com/t/future-of-chaos-physx-and-other-physics-engines/2610544

- Grenoville, F. **From One Car to a Fleet: Asynchronous Physics in UE5** — Level Up (Medium).
  https://levelup.gitconnected.com/from-one-car-to-a-fleet-asynchronous-physics-in-ue5-that-scales-with-multithreading-d322b2b94001

- Cai, I. **UE Physics Framework internals** (UE 5.1) — itscai.us.
  https://itscai.us/blog/post/ue-physics-framework/

---

## Corpos Rígidos e Colisões

- Epic Games. **Physics Bodies — Collision Settings** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-bodies-reference-for-unreal-engine

- Epic Games. **Set Use CCD** — UE 5.8 Blueprint API Reference.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Physics/SetUseCCD

- Epic Games. **Collision Overview** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-overview-in-unreal-engine

---

## Juntas e Restrições

- Epic Games. **Physics Constraint Reference** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/unreal-engine/physics-constraint-reference-in-unreal-engine

- Epic Games. **Physics Constraint Component User Guide** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/using-physics-constraints-in-unreal-engine

- Epic Games. **Applying a Physics Constraint Profile** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/applying-a-physics-constraint-profile-in-unreal-engine

- Epic Games. **Applying a Physical Animation Profile** — UE 5.7 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/applying-a-physical-animation-profile-in-unreal-engine

---

## Corpos Flexíveis (Soft Bodies / Cloth)

- Epic Games. **Chaos Cloth** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-cloth-in-unreal-engine

- Epic Games. **Chaos Cloth Updates 5.7** — Epic Dev Community Tutorial.
  https://dev.epicgames.com/community/learning/tutorials/1o0R/unreal-engine-chaos-cloth-updates-5-7

- Epic Games. **Chaos Flesh Overview** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-flesh-overview

- Epic Games. **Exploring Chaos: Flesh, Fluids, and Fractures** — Unreal Fest 2023 (talk).
  https://dev.epicgames.com/community/learning/talks-and-demos/Eopj/unreal-engine-exploring-chaos-flesh-fluids-and-fractures

---

## Destruição Dinâmica (Fracture)

- Epic Games. **Chaos Destruction in Unreal Engine** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-in-unreal-engine

- Epic Games. **Geometry Collections User Guide** — UE 5.7 Docs.
  https://dev.epicgames.com/documentation/unreal-engine/geometry-collections-user-guide

- Epic Games. **Fracturing Geometry Collections User Guide** — UE 5.7 Docs.
  https://dev.epicgames.com/documentation/unreal-engine/fracturing-geometry-collections-user-guide

- Epic Games. **Chaos Fields User Guide** — UE 5.8 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-fields-user-guide-in-unreal-engine

- Epic Games. **Chaos Destruction in Sequencer with Chaos Cache** — Epic Dev Community Tutorial.
  https://dev.epicgames.com/community/learning/tutorials/bX8J/unreal-engine-chaos-destruction-in-sequencer-with-chaos-cache-cinematic-playback-and-optimization

---

## Fluidos e Partículas Físicas

- Epic Games. **Niagara Fluids in Unreal Engine** — UE 5.7 Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/niagara-fluids-in-unreal-engine

- Epic Games. **Scene Interactions with Niagara Fluids** — Epic Dev Community Tutorial.
  https://dev.epicgames.com/community/learning/tutorials/8kkP/scene-interactions-with-niagara-fluids

- Zhu, A. (Epic Games). **Working with Niagara Fluids to Create Water Simulations** — 80.lv.
  https://80.lv/articles/working-with-niagara-fluids-to-create-water-simulations

---

## Arquitetura Base e Performance

- Lentine, M. (Epic Games). **Chaos Scene Queries and Rigid Body Engine in UE5** — Epic Tech Blog (maio 2022).
  https://www.unrealengine.com/en-US/tech-blog/chaos-scene-queries-and-rigid-body-engine-in-ue5

- Grenoville, F. **Asynchronous Physics in UE5** — Level Up (Medium).
  https://levelup.gitconnected.com/from-one-car-to-a-fleet-asynchronous-physics-in-ue5-that-scales-with-multithreading-d322b2b94001

- Cai, I. **UE Physics Framework internals — FPhysScene, FPBDRigidsSolver** (UE 5.1).
  https://itscai.us/blog/post/ue-physics-framework/

- Epic Games. **Large World Coordinates in Unreal Engine 5** — UE Docs.
  https://dev.epicgames.com/documentation/en-us/unreal-engine/large-world-coordinates-in-unreal-engine-5

- Intel. **Unreal Engine's New Chaos Physics System — Intel CPU Optimizations** (2022).
  Benchmarks de scene queries e throughput do Chaos vs PhysX no UE5.0.
  https://www.intel.com/content/dam/develop/external/us/en/documents/unreal-engines-new-chaos-physics-system-screams-with-in-depth-intel-cpu-optimizations.pdf

---

## Fontes Acadêmicas e Técnicas (GDC / SIGGRAPH / arXiv)

> Ver também `references-academic.md` para lista completa com resumos.

- Van Allen, J. et al. (Epic Games). **Causing Chaos: The Future of Physics and Destruction in Unreal Engine** — GDC 2019.
  Apresentação fundacional do Chaos Destruction: Field System, Geometry Collections, Anchor Fields, strain dinâmico.
  https://www.youtube.com/watch?v=6T8LzaIq3Qs

- Lentine, M. (Epic Games). **Chaos Physics in LEGO Fortnite: Building A Fully Interactive Sandbox Experience** — GDC 2024.
  Escala do Chaos Destruction para mundo aberto determinístico 95 km²; hierarchical clustering + rollback networking.
  https://www.youtube.com/watch?v=WPsRfZ8rxOg

- Balog, M. (Epic Games). **Dynamic Destruction in UE5 with the Chaos Destruction System** — GDC 2025.
  Otimizações custo-benefício para Geometry Collections no UE 5.5; workflows Dataflow não-destrutivos.
  https://www.unrealengine.com/events/gdc-2025

- Han, Y. et al. **A Neural Network Model for Efficient Musculoskeletal-Driven Skin Deformation** — SIGGRAPH 2024.
  Metodologia por trás do pipeline ML Deformer do Chaos Flesh: rede neural treinada em simulação tetraédrica offline.
  https://physicsbasedanimation.com/2024/07/27/a-neural-network-model-for-efficient-musculoskeletal-driven-skin-deformation/

- Chen, Y., Han, Y. et al. **Position-Based Nonlinear Gauss-Seidel for Quasistatic Hyperelasticity** — SIGGRAPH 2024.
  Solver que supera limitações de iteração do XPBD padrão para soft bodies volumétricos (Chaos Flesh).
  https://dl.acm.org/doi/10.1145/3658229

- Li, C. et al. **Unsmoothed Aggregation Algebraic Multigrid for XPBD** — SIGGRAPH 2025.
  AMG+PCG para XPBD de alta resolução; resolve falhas de convergência do solver Chaos em soft bodies densos.
  https://www.siggraph.org/wp-content/uploads/2025/08/Conference-Papers.html

- Macklin, M. et al. **Small Steps in Physics Simulation** — SIGGRAPH/SCA 2019.
  Base matemática de substeps; fundação algorítmica do solver XPBD usado no Chaos Physics.
  https://mmacklin.com/smallsteps.pdf

- Sun, Y. et al. **Leapfrog Flow Maps for Real-Time Fluid Simulation** — SIGGRAPH 2025.
  AMGPCG GPU para fluidos incompressíveis com vórtices em tempo real; diretamente aplicável ao roadmap de Niagara Fluids.
  https://yuchen-sun-cg.github.io/projects/lfm/static/pdfs/SIG_2025_Leapfrog_Flow_Maps.pdf
