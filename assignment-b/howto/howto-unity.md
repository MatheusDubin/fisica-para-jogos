# How-To: Unity — Setup, Profiling e Métricas

> Guia prático para quem está saindo do zero ou está enferrujado.
> Versão de referência: Unity 6 LTS (ou 2022 LTS — mesmos passos).
> Path usado: **GameObject + PhysX** (não DOTS).

---

## 1. Criar o Projeto

1. Abrir **Unity Hub**
2. Clicar em **New Project**
3. Selecionar template: **3D (URP)** ou **3D (Built-in)** — ambos funcionam; Built-in é mais simples para benchmark
4. Nomear: `project-f-unity`
5. Criar

---

## 2. Configurar Fixed Timestep (obrigatório)

`Edit > Project Settings > Time`

- **Fixed Timestep:** `0.02` (50 Hz)
- **Maximum Allowed Timestep:** `0.1` (deixar padrão)

Confirmar que ficou `0.02` — é o que garante comparação justa com as outras engines.

---

## 3. Configurar Physics

`Edit > Project Settings > Physics`

Antes de qualquer coisa, **anotar e salvar** os valores padrão:


| Configuração                       | Valor padrão (anotar aqui) |
| ---------------------------------- | -------------------------- |
| Default Solver Iterations          | 6                          |
| Default Solver Velocity Iterations | 1                          |
| Sleep Threshold                    | 0.005                      |
| Default Contact Offset             | 0.01                       |


> **Não alterar nenhum desses valores.** Estamos medindo o comportamento padrão da engine.

- **Gravity:** confirmar `(0, -9.81, 0)`
- **Default Solver Iterations:** padrão é 6 — anotar
- **Sleep Threshold:** padrão é 0.005 — anotar

---

## 4. Abrir o Profiler Window

`Window > Analysis > Profiler` (ou `Ctrl+7`)

O Profiler é a ferramenta principal para medir o custo de física. Antes de entender o código, vale entender a UI:

- **CPU Usage track** — linha azul: mostra o tempo total por frame
- **Physics track** — linha verde: mostra o tempo que o PhysX gastou naquele frame
- Clicar em qualquer frame no gráfico mostra o breakdown na metade inferior

Para ver apenas física: desativar todas as tracks exceto `Physics` clicando no ícone de olho de cada uma.

---

## 5. Medir Physics Step Time via Código (ProfilerRecorder)

Esta é a forma correta de capturar o tempo de física **programaticamente** para salvar em CSV.

```csharp
using UnityEngine;
using Unity.Profiling;

public class PhysicsMetrics : MonoBehaviour
{
    private ProfilerRecorder physicsRecorder;

    void OnEnable()
    {
        // Iniciar gravação do marcador interno do PhysX
        physicsRecorder = ProfilerRecorder.StartNew(
            ProfilerCategory.Physics,
            "Physics.Processing"  // nome do marcador interno
        );
    }

    void OnDisable()
    {
        physicsRecorder.Dispose();
    }

    void FixedUpdate()
    {
        // LastValue retorna nanosegundos (long)
        long ns = physicsRecorder.LastValue;
        float ms = ns / 1_000_000f;  // converter para milissegundos

        // usar o valor ms para registrar
        Debug.Log($"Physics Step Time: {ms:F3} ms");
    }
}
```

> **Se `Physics.Processing` não funcionar:** abrir o Profiler Window durante play mode, ir até a track Physics, e ver qual é o nome exato do marcador que aparece. Alternativas: `"Physics.Simulate"`, `"PhysicsManager.FixedUpdate"`.

**Por que usar FixedUpdate e não Update?**
`FixedUpdate` roda em sincronia com o passo de física — é o lugar certo para ler a métrica do passo que acabou de ocorrer.

---

## 6. Medir FPS

```csharp
void Update()
{
    // unscaledDeltaTime ignora Time.timeScale — mais preciso para benchmark
    float fps = 1f / Time.unscaledDeltaTime;
}
```

Adicionar no `Start()` para remover o cap de FPS e o V-Sync:

```csharp
void Start()
{
    Application.targetFrameRate = -1;   // sem cap
    QualitySettings.vSyncCount = 0;     // sem V-Sync
}
```

---

## 7. Detectar Sleep State (Cenário 1 — Torre)

```csharp
using UnityEngine;

public class SleepDetector : MonoBehaviour
{
    public Rigidbody[] todosOsCorpos;
    private float tempoInicio;
    private bool medindo = true;

    void Start()
    {
        tempoInicio = Time.time;
    }

    void FixedUpdate()
    {
        if (!medindo) return;

        int dormindo = 0;
        foreach (var rb in todosOsCorpos)
        {
            if (rb.IsSleeping()) dormindo++;
        }

        if (dormindo == todosOsCorpos.Length)
        {
            float tempoAteSleep = Time.time - tempoInicio;
            Debug.Log($"Todos dormiram em: {tempoAteSleep:F2}s");
            medindo = false;
            // → salvar em CSV aqui
        }
    }
}
```

**Como popular `todosOsCorpos`:** no script de spawn, após criar cada `Rigidbody`, adicionar à lista:

```csharp
List<Rigidbody> corpos = new List<Rigidbody>();

for (int i = 0; i < numCubos; i++)
{
    GameObject cubo = Instantiate(cuboPrefab, posicao, Quaternion.identity);
    corpos.Add(cubo.GetComponent<Rigidbody>());
}

todosOsCorpos = corpos.ToArray();
```

---

## 8. Salvar Dados em CSV

```csharp
using System.IO;

public static void SalvarCSV(string caminho, string linha)
{
    // Cria o arquivo se não existir; adiciona linha ao final se existir
    using (StreamWriter writer = new StreamWriter(caminho, append: true))
    {
        writer.WriteLine(linha);
    }
}

// Uso:
// SalvarCSV("Assets/Data/torre_unity.csv", $"{runAtual},{tempoAteSleep}");
// SalvarCSV("Assets/Data/chuva_unity.csv", $"{runAtual},{variacao},{stepTimeMs},{fps}");
```

Para criar o header na primeira run:

```csharp
if (runAtual == 0)
    SalvarCSV(caminho, "run,variacao,physics_step_ms,fps");
```

---

## 9. Automatizar 10 Runs (Reiniciar Cena)

Usar um **singleton persistente** (não é destruído ao recarregar a cena):

```csharp
// RunManager.cs — adicionar a um GameObject e marcar DontDestroyOnLoad
using UnityEngine;
using UnityEngine.SceneManagement;

public class RunManager : MonoBehaviour
{
    public static RunManager Instance;
    public int runAtual = 0;
    public int totalRuns = 10;

    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);  // sobrevive ao reload de cena
        }
        else
        {
            Destroy(gameObject);  // garante singleton único
        }
    }

    public void ProximaRun()
    {
        runAtual++;
        if (runAtual >= totalRuns)
        {
            Debug.Log("Benchmark concluído!");
            // encerrar ou processar resultados finais
            return;
        }
        // Recarregar a cena atual
        SceneManager.LoadScene(SceneManager.GetActiveScene().name);
    }
}
```

Chamar `RunManager.Instance.ProximaRun()` ao final de cada medição.

---

## 10. Configurar Objetos Físicos

### Cubo (Cenário 1 — Torre)

Criar um Prefab com:

- `GameObject > 3D Object > Cube` (scale = 1,1,1 = 1m³)
- Adicionar componente `Rigidbody`
  - Mass: `1`
  - Drag: `0`
  - Angular Drag: `0.05` (padrão)
  - Collision Detection: `Discrete` (padrão — não mudar)
- O `BoxCollider` já vem junto com o Cube — não remover
- Criar um `PhysicMaterial`:
  - `Assets > Create > Physics Material`
  - Dynamic Friction: `0.4`
  - Static Friction: `0.5`
  - Bounciness: `0.0` ← crítico para a Torre
  - Friction Combine: `Average`
  - Bounce Combine: `Minimum`
  - Arrastar o material para o campo `Material` do `BoxCollider`

### Esfera (Cenário 2 — Chuva)

- `GameObject > 3D Object > Sphere` (scale = 1,1,1 = diâmetro 1m, raio 0.5m)
- Adicionar `Rigidbody` com mesmos parâmetros
- `SphereCollider` já vem junto
- `PhysicMaterial` com Bounciness: `0.3` para a Chuva

### Chão e Funil (Static)

- `GameObject > 3D Object > Plane` para o chão
  - Não adicionar `Rigidbody` — deixar estático
  - Adicionar o mesmo `PhysicMaterial`
- Para as paredes do funil: usar `GameObject > 3D Object > Cube`, escalar e rotacionar, sem `Rigidbody`

---

## 11. Checklist antes de Rodar

- Fixed Timestep = 0.02 confirmado
- V-Sync desativado (`QualitySettings.vSyncCount = 0` no código)
- Frame Rate cap removido (`Application.targetFrameRate = -1`)
- Bounciness = 0.0 no PhysicMaterial dos cubos (Torre)
- Valores padrão do solver anotados (não alterados)
- ProfilerRecorder inicializado no `OnEnable()`
- RunManager criado e presente na cena
- CSV sendo gravado com header na primeira run
- Build em **Release** (não Debug) — `File > Build Settings > Development Build` **desmarcado**

---

## 12. Referência Rápida de APIs


| O que fazer               | API                                                          |
| ------------------------- | ------------------------------------------------------------ |
| Tempo de física (ms)      | `ProfilerRecorder.LastValue / 1_000_000f`                    |
| FPS                       | `1f / Time.unscaledDeltaTime`                                |
| Corpo está dormindo?      | `rigidbody.IsSleeping()`                                     |
| Acordar corpo manualmente | `rigidbody.WakeUp()`                                         |
| Tempo de simulação        | `Time.time` (segundos desde o início)                        |
| Recarregar cena           | `SceneManager.LoadScene(SceneManager.GetActiveScene().name)` |
| Salvar arquivo            | `System.IO.StreamWriter` com `append: true`                  |
| Desativar V-Sync          | `QualitySettings.vSyncCount = 0`                             |
| Remover frame cap         | `Application.targetFrameRate = -1`                           |


