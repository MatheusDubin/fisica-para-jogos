# Assignment A — Physics Engines Survey (Grau A)

## Entrega
- **Formato:** Apresentação de slides exportada como PDF
- **Canal:** Espaço designado no Moodle
- **Tipo:** Pesquisa teórica + tabela comparativa

---

## Enunciado Original

### Contexto
Historicamente, as ferramentas de física nos motores de jogos dependiam fortemente de bibliotecas de terceiros (como as versões antigas do PhysX ou Havok) focadas quase exclusivamente em corpos rígidos. Nos últimos anos, acompanhando a evolução do hardware e o aumento das exigências por realismo, os grandes motores de jogos (Unity, Unreal e Godot) passaram por profundas reformulações arquitetônicas em seus sistemas de física, introduzindo soluções de larga escala, processamento paralelo e simulações complexas.

### Objetivo
Realizar um levantamento do **estado da técnica** dos atuais motores de física (Physics Engines) integrados às três principais Game Engines do mercado: Unity, Unreal Engine e Godot. Investigar tecnologias vigentes, comparar recursos estruturais e entender como lidam com problemas modernos de simulação física em tempo real.

### Estrutura Obrigatória dos Slides
1. **Introdução** — Objetivo da pesquisa e contexto das engines.
2. **Panorama Atual** — Motores físicos subjacentes em cada engine.
3. **Análise Comparativa** — Dados da tabela e discussão de pontos fortes/fracos.
4. **Considerações Finais** — Reflexão sobre o futuro das simulações.
5. **Referências** — Documentações e bibliografia utilizada.

---

## Tabela Comparativa (Obrigatória)

> Atenção: não basta Sim/Não — especificar o **nome do sistema ou componente** utilizado.

| Recurso / Estrutura Física | Unity (sistemas atuais) | Unreal Engine (sistema atual) | Godot (sistemas atuais) |
|---|---|---|---|
| **Corpos Rígidos e Colisões** | Como lida com colisões contínuas vs discretas? | | |
| **Juntas e Restrições** | Principais tipos de juntas suportados? | | |
| **Corpos Flexíveis (Soft Bodies / Cloth)** | Suporte nativo volumétrico ou apenas tecido? | | |
| **Destruição Dinâmica (Fracture)** | Real-time procedural ou pré-calculada? | | |
| **Fluidos e Partículas Físicas** | Como simula líquidos interagindo com sólidos? | | |
| **Arquitetura Base e Performance** | ECS, DOTS, aceleração por GPU? | | |

---

## Plano de Execução

### Fase 1 — Pesquisa por Engine (Agente de Pesquisa)
Objetivo: preencher a tabela com fontes primárias por engine.

#### Unity
- [ ] Identificar versão atual padrão (PhysX 4.x vs Havok via pacote)
- [ ] Checar suporte a Cloth (Cloth component) e Soft Body (DOTS Physics?)
- [ ] Investigar Destruction / Fracture no contexto do Unity
- [ ] Arquitetura: Unity DOTS, Physics for DOTS (Unity.Physics), ECS
- [ ] Fontes: `docs.unity3d.com/Manual/PhysicsSection.html`, Unity Blog, fóruns

#### Unreal Engine
- [ ] Confirmar Chaos Physics como padrão (UE5+), status do Chaos Cloth, Chaos Flesh
- [ ] Investigar Chaos Destruction (Geometry Collections / Fracture Tool)
- [ ] Fluidos: Niagara Fluids, Chaos Fluids
- [ ] Arquitetura: large world coordinates, multithreading, GPU simulation
- [ ] Fontes: `docs.unrealengine.com`, Unreal Dev Community, GDC talks

#### Godot
- [ ] Confirmar status do Godot Physics vs Jolt Physics (disponível desde Godot 4.x)
- [ ] Verificar suporte a Soft Body (SoftBody3D node), Cloth
- [ ] Destruição: procedural ou workaround?
- [ ] Fluidos: GPUParticles, ausência de solver dedicado?
- [ ] Arquitetura: GDNative, single-threaded vs multi-threaded
- [ ] Fontes: `docs.godotengine.org`, repositório GitHub, proposta de Jolt

### Fase 2 — Artigos Acadêmicos e Técnicos
- [ ] Google Scholar: "Unity physics benchmark", "Chaos Physics Unreal", "Jolt physics engine"
- [ ] SBGames anais: buscar papers sobre simulação física em jogos brasileiros
- [ ] CAPES (via proxy Unisinos): IEEE Xplore, Scopus para artigos mais formais

### Fase 3 — Síntese e Slides
- [ ] Preencher tabela comparativa com dados encontrados (+ citar fontes por célula)
- [ ] Identificar 2-3 pontos fortes e fracos por engine
- [ ] Redigir seção "Considerações Finais" com tendências (GPU physics, ECS, open-source)
- [ ] Montar slides no formato solicitado (5 seções)
- [ ] Exportar PDF

---

## Fontes Sugeridas

| Fonte | URL / Localização |
|---|---|
| Unity Physics Manual | https://docs.unity3d.com/Manual/PhysicsSection.html |
| Unity DOTS Physics | https://docs.unity3d.com/Packages/com.unity.physics@latest |
| Unreal Chaos Physics | https://docs.unrealengine.com/5.0/en-US/chaos-physics-overview/ |
| Unreal Chaos Destruction | https://docs.unrealengine.com/5.0/en-US/chaos-destruction-overview/ |
| Godot Physics docs | https://docs.godotengine.org/en/stable/tutorials/physics/ |
| Jolt Physics (Godot addon) | https://github.com/godot-jolt/godot-jolt |
| SBGames Anais | https://www.sbgames.org/edicoes-anteriores/ |
| Google Scholar | https://scholar.google.com |
| Portal CAPES | https://www.periodicos.capes.gov.br |

---

## Notas e Decisões

> Use esta seção para registrar achados importantes durante a pesquisa.

- _[a preencher durante pesquisa]_

---

## Status
- [ ] Fase 1 — Pesquisa por engine
- [ ] Fase 2 — Artigos acadêmicos
- [ ] Fase 3 — Síntese e slides
- [ ] Entrega no Moodle
