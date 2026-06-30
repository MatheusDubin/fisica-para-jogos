# Referências Acadêmicas e Técnicas — Physics Engines (Assignment A)

> Fontes acadêmicas (conferências, periódicos, GDC talks) organizadas pelos 6 tópicos da tabela comparativa.
> Coletadas via Google Deep Research — junho 2026.
> Complementam as referências de documentação oficial em `references-unity.md`, `references-unreal.md`, `references-godot.md`.

---

## Arquitetura Base e Performance

- Rouwé, J. (Guerrilla Games / Sony). **Architecting Jolt Physics for 'Horizon Forbidden West'** — GDC 2022.
  Detalha a arquitetura de 32 RW mutexes do Jolt, island building lock-free, e eliminação de thread stalls em mundo aberto. Redução de stall de 500 µs → 10 µs por thread; tick rate dobrado de 30 Hz → 60 Hz; redução de 25% no footprint de memória. Adotado no Godot 4.4+.
  https://gdcvault.com/play/1027560/Architecting-Jolt-Physics-for-Horizon
  (PDF de notas): https://jrouwe.nl/architectingjolt/ArchitectingJoltPhysics_Rouwe_Jorrit_Notes.pdf

- Balakshin, A. **ECS in Practice: The Case of Unity** — GDC 2024.
  Implementação prática do DOTS e física stateless em produção (Alan Wake 2); análise do impacto do Burst Compiler em velocidade de execução e escalabilidade multi-thread.
  https://gdcvault.com/play/1034295/ECS-in-Practice-The-Case

- Oliveira, S. et al. **Towards Scalable Cloud Gaming Systems: Decoupling Physics from the Game Engine** — SBGames 2023.
  Propõe e avalia arquitetura distribuída para offloading de física para VMs em nuvem; demonstra desempenho superior em cenários computacionalmente intensos.
  https://sol.sbc.org.br/index.php/sbgames/article/view/27678

---

## Corpos Rígidos e Solvers

- Macklin, M., Storey, K. et al. **Small Steps in Physics Simulation** — ACM SIGGRAPH / Symposium on Computer Animation (SCA) 2019.
  Demonstra formalmente que dividir o frame em múltiplos substeps isométricos melhora dramaticamente a estabilidade e rigidez do solver com overhead mínimo vs. aumentar iterações em PGS. Base matemática para o solver com substeps em Unity Physics e Chaos (XPBD).
  https://mmacklin.com/smallsteps.pdf
  (Semantic Scholar): https://www.semanticscholar.org/paper/Small-steps-in-physics-simulation-Macklin-Storey/7dd777c0c51d4d2682836fdb6420cc634b234664

- Macklin, M., Müller, M., Chentanez, N. **Non-smooth Newton Methods for Deformable Multi-body Dynamics** — ACM Transactions on Graphics (TOG) 2019.
  Framework altamente estável para corpos rígidos e deformáveis usando iterações não-suaves de Newton para resolver o Nonlinear Complementarity Problem (NCP) diretamente.
  https://mmacklin.com/

- Chentanez, N. et al. (NVIDIA). **Game Physics on the GPU** — GDC 2018.
  Descreve a migração de broadphase, geração de contato por distância e solvers PGS/TGS para GPU compute no PhysX. Fundação para PhysX 4 TGS e GPU acceleration.
  https://gdcvault.com/play/1024345/Game-Physics-on-the-GPU

- NVIDIA. **PhysX 4: Raising the Fidelity and Performance of Physics Simulation in Games** — GTC 2019 (apresentação técnica).
  Introduz o solver Temporal Gauss-Seidel (TGS) no PhysX 4: atualização posicional durante iterações vs. Baumgarte apenas no final do frame (PGS clássico). Resolve instabilidade em constraint chains e mass ratios altos.
  https://developer.download.nvidia.com/video/gputechconf/gtc/2019/presentation/s9990-physx-4-raising-the-fidelity-and-performance-of-physics-simulation-in-games.pdf

---

## Juntas, Restrições e Articulações

- Kaup, M. et al. **A Review of Nine Physics Engines for Reinforcement Learning Research** — arXiv 2024.
  Avalia comparativamente MuJoCo, PhysX, Unity, PyBullet e outros. Conclui que engines com solvers de coordenadas reduzidas (Featherstone / articulações minimais) superam engines de jogos tradicionais em fidelidade para RL. Relevante para posicionar Unity (ArticulationBody) e PhysX 5.
  https://arxiv.org/abs/2407.08590

- Mukai, T. **Transformation Constraints Using Approximate Spherical Regression** — Journal of Computer Graphics Techniques (JCGT) 2022.
  Modelos matemáticos avançados para constraints de transformação rotacional; aplicável a como engines modernas resolvem mecânica de joints complexos.
  https://mukai-lab.org/publications/

- Anonymous. **Taccel: Scaling Up Vision-based Tactile Robotics via High-performance GPU Simulation** — arXiv 2025.
  Utiliza forward dynamics de coordenadas reduzidas e articulação controllers dentro de engines de física para modelar contato de alta frequência. Contextualiza convergência física de jogos / robótica.
  https://arxiv.org/html/2504.12908v2

---

## Corpos Flexíveis (Soft Bodies / Cloth)

- Han, Y. et al. (Yushan Han, Univ. of Southern California). **A Neural Network Model for Efficient Musculoskeletal-Driven Skin Deformation** — ACM SIGGRAPH 2024.
  Metodologia por trás da simulação volumétrica de tecidos moles em tempo real: rede neural treinada em dados de simulação hiperplástica de tetraedros para aproximar deformação muscular e deslizamento de pele. Base técnica do pipeline ML Deformer do Chaos Flesh (Unreal).
  https://physicsbasedanimation.com/2024/07/27/a-neural-network-model-for-efficient-musculoskeletal-driven-skin-deformation/
  (página do autor): https://yushanh.github.io/publications/

- Chen, Y., Han, Y. et al. **Position-Based Nonlinear Gauss-Seidel for Quasistatic Hyperelasticity** — ACM SIGGRAPH 2024.
  Solver inovador para materiais hiperplásticos quasistáticos; supera as limitações de iteração do XPBD padrão para soft bodies volumétricos de alta densidade. Relevante para Chaos Flesh e Unity Physics volumétrico.
  https://dl.acm.org/doi/10.1145/3658229

- Li, C. et al. **Unsmoothed Aggregation Algebraic Multigrid for XPBD** — ACM SIGGRAPH 2025.
  Introduz método AMG (Algebraic Multigrid) com Preconditioned Conjugate Gradient (PCG) para resolver falhas de convergência do XPBD padrão em soft bodies de alta resolução e alta rigidez. Avanço direto sobre as limitações do solver Chaos (UE) e Unity Physics.
  https://www.siggraph.org/wp-content/uploads/2025/08/Conference-Papers.html

- Bournemouth University. **State-of-the-art Improvements and Applications of Position Based Dynamics** — 2022.
  Survey acadêmico completo sobre PBD/XPBD: histórico, variantes, aplicações em jogos e simulação. Excelente referência de contextualização.
  https://eprints.bournemouth.ac.uk/38303/1/State-of-the-art%20Improvements%20and%20Applications%20of%20Position%20Based%20Dynamics_v11_05_12_2022.pdf

---

## Destruição Dinâmica (Fracture)

- Van Allen, J. et al. (Epic Games). **Causing Chaos: The Future of Physics and Destruction in Unreal Engine** — GDC 2019.
  Apresentação fundacional do Chaos Destruction: Field System, Geometry Collections, Anchor Fields, avaliação dinâmica de strain, substituição do PhysX para destruição. Primeira demonstração pública do sistema que se tornaria padrão no UE5.
  https://www.youtube.com/watch?v=6T8LzaIq3Qs
  (GDC Vault): https://www.unrealengine.com/tech-blog/unreal-engine-gdc-2019-tech-talks-now-available-online

- Lentine, M. (Epic Games). **Chaos Physics in LEGO Fortnite: Building A Fully Interactive Sandbox Experience** — GDC 2024.
  Detalha as técnicas de networking, hierarchical clustering e otimização necessárias para escalar o Chaos Destruction a um mundo aberto totalmente determinístico de 95 km² com milhares de rigid bodies sincronizados em multiplayer.
  https://www.youtube.com/watch?v=WPsRfZ8rxOg

- Balog, M. (Epic Games). **Dynamic Destruction in UE5 with the Chaos Destruction System** — GDC 2025.
  Técnicas de custo-benefício no UE 5.5 para destruição de Geometry Collections sem uso excessivo de Fields; workflows não-destrutivos com Dataflow.
  https://www.unrealengine.com/events/gdc-2025

---

## Fluidos e Partículas Físicas

- Sun, Y. et al. **Leapfrog Flow Maps for Real-Time Fluid Simulation** — ACM SIGGRAPH 2025.
  Introduz LFM e solver AMGPCG matrix-free para GPU, permitindo fluidos incompressíveis com vórtices complexos em tempo real. Abre caminho para integração em frameworks como Niagara Fluids (Unreal). Demonstra fenômenos aerodinâmicos (vórtices de asa delta, bolas de fogo) a taxas interativas.
  https://yuchen-sun-cg.github.io/projects/lfm/static/pdfs/SIG_2025_Leapfrog_Flow_Maps.pdf
  (página do projeto): https://yuchen-sun-cg.github.io/projects/lfm/

- Anonymous. **Adaptive Phase-Field-FLIP for Very Large Scale Two-Phase Fluid Simulation** — ACM SIGGRAPH 2025.
  Método híbrido Euleriano/Lagrangiano para simular fluidos multifásicos turbulentos (água + ar gerando espuma) com base física, substituindo heurísticas de VFX para efeitos como wake de barcos e cachoeiras.
  https://www.siggraph.org/wp-content/uploads/2025/08/Conference-Papers.html

- Anonymous. **Large Viscoelastic Fluid Simulation on GPU** — SBGames 2017.
  Aceleração CUDA de SPH para fluidos viscoelásticos não-Newtonianos (lama, muco, gelatina); 7,76× mais rápido que OpenMP CPU; até 1 milhão de partículas em tempo real com acoplamento fluido-sólido. Contexto direto para ausência de solver SPH nativo em Unity/Godot.
  https://www.sbgames.org/sbgames2017/papers/ComputacaoFull/175135.pdf

---

## Comparação Transversal de Engines

- SBGames 2011. **A Rigid Body Physics Engine for Interactive Applications** — SBGames 2011.
  Paper fundacional brasileiro sobre implementação de engine de física rígida para aplicações interativas; contexto histórico relevante para a evolução até o estado atual.
  https://www.sbgames.org/sbgames2011/proceedings/sbgames/papers/comp/short/02-91515_2.pdf

- Intel. **Unreal Engine's New Chaos Physics System Screams With In-Depth Intel CPU Optimizations** — Intel Developer (2022).
  Análise de performance do Chaos vs PhysX em hardware Intel; benchmarks de scene queries e rigid body throughput no UE5.0.
  https://www.intel.com/content/dam/develop/external/us/en/documents/unreal-engines-new-chaos-physics-system-screams-with-in-depth-intel-cpu-optimizations.pdf
