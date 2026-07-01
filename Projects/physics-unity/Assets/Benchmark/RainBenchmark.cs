using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Benchmark
{
    // Cenario 2 - A Chuva (in-place, sem scene reload)
    // -------------------------------------------------------------------------
    // Ambiente (camera, luz, caixa fechada 35x80x35) criado UMA VEZ no Start.
    // Para cada variacao (1000, 5000, 10000) e cada run (1..10):
    //   1. Spawna N esferas num grid 3D com jitter +-3cm (seed = 0xBEEF + run).
    //   2. Aquecimento de 0.5s (descartado).
    //   3. Janela de coleta de 10s (wall-clock): a cada FixedUpdate cronometra
    //      Physics.Simulate() (SimulationMode.Script) com Stopwatch = Physics
    //      Step Time; a cada Update grava FPS (1/unscaledDeltaTime).
    //   4. Calcula media/max do step e media do FPS, grava no CSV.
    //   5. Reset in-place (destroi esferas, espera frames) e proxima run.
    //
    // amostras = numero de FixedUpdate na janela. Se < 500, a simulacao rodou
    // abaixo de 50Hz (spiral of death) - diagnostico importante em 10k.
    //
    // A janela e medida em WALL-CLOCK (unscaledTime) de proposito: se o physics
    // nao consegue manter 50Hz, menos passos cabem nos 10s reais -> amostras<500.
    public class RainBenchmark : MonoBehaviour
    {
        const float SPHERE_SCALE = 1.0f;  // esfera primitiva Unity tem diametro 1 (raio 0.5)
        const float SPACING = 1.15f;      // > diametro, evita overlap inicial
        const float SPAWN_Y = 25.0f;      // altura de spawn acima do chao da caixa
        int _totalRuns = 10;              // configuravel via bench_rain_runs / BENCH_RAIN_RUNS
        // Janela por TEMPO DE SIMULACAO (contagem de FixedUpdate = passos de
        // fisica), NAO wall-clock: garante que todas as engines medem o MESMO
        // estado fisico (queda + pilha) na mesma duracao simulada.
        const int WARMUP_STEPS = 25;   // ~0.5s simulados (curto de proposito)
        const int WINDOW_STEPS = 500;  // 10s simulados - a correcao de verdade
        int _steps = 0;

        const string SUBDIR = "chuva-default";
        const string CSV = "chuva_unity.csv";
        const string CSV_HEADER = "run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras";

        int[] _variacoes = { 1000, 5000, 10000 };

        PhysicsMaterial _sharedMat;
        readonly List<Rigidbody> _spheres = new List<Rigidbody>();

        int _varIdx = 0;
        int _run = 0;      // 0-based dentro da variacao
        int _numObjetos = 0;

        bool _aquecendo, _coletando, _running;
        double _tStart, _tColeta;
        readonly List<float> _stepMs = new List<float>();
        readonly List<float> _fps = new List<float>();

        string _csvPath;

        void OnDisable()
        {
            // Restaura o modo automatico (resetaria sozinho ao sair do Play, mas
            // explicito e mais seguro).
            Physics.simulationMode = SimulationMode.FixedUpdate;
        }

        void Start()
        {
            // Stepping manual para cronometrar o passo de fisica com precisao.
            Physics.simulationMode = SimulationMode.Script;

            // Permite rodar uma unica variacao via env (ex: BENCH_RAIN_VARIATIONS=10000).
            string ov = Environment.GetEnvironmentVariable("BENCH_RAIN_VARIATIONS");
            if (!string.IsNullOrEmpty(ov))
            {
                var parts = ov.Split(',');
                var list = new List<int>();
                foreach (var p in parts)
                    if (int.TryParse(p.Trim(), out int v) && v > 0) list.Add(v);
                if (list.Count > 0) _variacoes = list.ToArray();
            }

            string runsRaw = Environment.GetEnvironmentVariable("BENCH_RAIN_RUNS");
            if (string.IsNullOrEmpty(runsRaw)) runsRaw = PlayerPrefs.GetString("bench_rain_runs", "10");
            if (int.TryParse(runsRaw, out int rr) && rr > 0) _totalRuns = rr;

            _csvPath = BenchmarkCommon.PrepareFile(SUBDIR, CSV);
            BenchmarkCommon.WriteHeader(_csvPath, CSV_HEADER);

            _sharedMat = new PhysicsMaterial("chuva")
            {
                dynamicFriction = 0.4f,
                staticFriction = 0.4f,
                bounciness = 0.3f,
                frictionCombine = PhysicsMaterialCombine.Average,
                bounceCombine = PhysicsMaterialCombine.Average,
            };

            CriarCaixa();
            BenchmarkCommon.CreateViewer("ChuvaViewer", new Vector3(55, 35, 55), new Vector3(0, 18, 0), 1000f);

            Debug.Log($"[Chuva] Unity {Application.unityVersion}, solverIter={Physics.defaultSolverIterations}, " +
                      $"simulationMode={Physics.simulationMode}, runs={_totalRuns}, variacoes=[{string.Join(",", _variacoes)}]");

            IniciarProximaRun();
        }

        void CriarCaixa()
        {
            const float largura = 35f, altura = 80f, profundidade = 35f, esp = 2f;
            float meioY = altura * 0.5f;
            var root = new GameObject("Caixa");

            // So o chao e visivel; paredes e teto sao collision-only (igual ao
            // Godot), para a camera externa ver as esferas e o FPS refletir
            // "chao + esferas" - o mesmo conjunto renderizado pelo Godot.
            Parede(root, new Vector3(0, -esp * 0.5f, 0), new Vector3(largura, esp, profundidade), true);
            Parede(root, new Vector3(0, altura + esp * 0.5f, 0), new Vector3(largura, esp, profundidade), false);
            Parede(root, new Vector3(largura * 0.5f + esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade), false);
            Parede(root, new Vector3(-largura * 0.5f - esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade), false);
            Parede(root, new Vector3(0, meioY, profundidade * 0.5f + esp * 0.5f), new Vector3(largura, altura, esp), false);
            Parede(root, new Vector3(0, meioY, -profundidade * 0.5f - esp * 0.5f), new Vector3(largura, altura, esp), false);
        }

        void Parede(GameObject parent, Vector3 pos, Vector3 size, bool visible)
        {
            var go = GameObject.CreatePrimitive(PrimitiveType.Cube);
            go.name = "Parede";
            go.transform.SetParent(parent.transform);
            go.transform.position = pos;
            go.transform.localScale = size;
            go.GetComponent<Collider>().sharedMaterial = _sharedMat;
            go.isStatic = true;
            if (!visible)
            {
                var mr = go.GetComponent<MeshRenderer>();
                if (mr != null) Destroy(mr);
            }
        }

        void IniciarProximaRun()
        {
            _numObjetos = _variacoes[_varIdx];
            _stepMs.Clear();
            _fps.Clear();
            _steps = 0;
            _aquecendo = true;
            _coletando = false;
            _running = true;

            float tSpawn = Time.realtimeSinceStartup;
            SpawnEsferas();
            _tStart = Time.unscaledTimeAsDouble;

            Debug.Log($"[Chuva] Variacao={_numObjetos}, Run {_run + 1}/{_totalRuns} " +
                      $"(spawn {_numObjetos} esferas em {(Time.realtimeSinceStartup - tSpawn) * 1000f:F0}ms)");
        }

        void SpawnEsferas()
        {
            int porLado = Mathf.Max(1, Mathf.CeilToInt(Mathf.Pow(_numObjetos, 1f / 3f)));
            float origemX = -(porLado - 1) * SPACING * 0.5f;
            float origemZ = -(porLado - 1) * SPACING * 0.5f;

            var rng = new System.Random(0xBEEF + _run); // seed deterministico por run

            var root = new GameObject($"Esferas_v{_numObjetos}_r{_run + 1}");
            int n = 0;
            for (int ix = 0; ix < porLado && n < _numObjetos; ix++)
                for (int iy = 0; iy < porLado && n < _numObjetos; iy++)
                    for (int iz = 0; iz < porLado && n < _numObjetos; iz++)
                    {
                        if (n >= _numObjetos) break;
                        var jitter = new Vector3(
                            (float)(rng.NextDouble() * 0.06 - 0.03),
                            (float)(rng.NextDouble() * 0.06 - 0.03),
                            (float)(rng.NextDouble() * 0.06 - 0.03));
                        var pos = new Vector3(
                            origemX + ix * SPACING,
                            SPAWN_Y + iy * SPACING,
                            origemZ + iz * SPACING) + jitter;
                        CriarEsfera(root.transform, pos);
                        n++;
                    }
        }

        void CriarEsfera(Transform parent, Vector3 pos)
        {
            var go = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            go.transform.SetParent(parent);
            go.transform.position = pos;
            go.transform.localScale = Vector3.one * SPHERE_SCALE;
            go.GetComponent<Collider>().sharedMaterial = _sharedMat;

            // Renderiza cada esfera (como o Godot fez) para que o FPS reflita um
            // custo de pipeline comparavel. O SRP Batcher do URP (ligado por
            // padrao) mantem o custo de CPU baixo com o material compartilhado.
            var rend = go.GetComponent<MeshRenderer>();
            rend.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;
            rend.receiveShadows = false;

            var rb = go.AddComponent<Rigidbody>();
            rb.mass = 1.0f;
            _spheres.Add(rb);
        }

        void Update()
        {
            if (_coletando)
                _fps.Add(1f / Time.unscaledDeltaTime);
        }

        void FixedUpdate()
        {
            if (!_running) return;

            // Passo de fisica MANUAL e cronometrado. Em SimulationMode.Script o
            // Unity nao avanca a fisica sozinho - nos chamamos Physics.Simulate
            // a cada FixedUpdate e medimos o tempo com Stopwatch (alta resolucao).
            // Isso e o "Physics Step Time" do spec, equivalente ao
            // TIME_PHYSICS_PROCESS do Godot, e NUNCA retorna 0 (ao contrario do
            // ProfilerRecorder("Physics.Processing"), que nao existe no Unity 6).
            var sw = System.Diagnostics.Stopwatch.StartNew();
            Physics.Simulate(Time.fixedDeltaTime);
            sw.Stop();
            float stepMs = (float)sw.Elapsed.TotalMilliseconds;

            _steps++;

            if (_aquecendo)
            {
                if (_steps >= WARMUP_STEPS)
                {
                    _aquecendo = false;
                    _coletando = true;
                }
                return;
            }

            if (_coletando)
            {
                _stepMs.Add(stepMs);

                if (_stepMs.Count % 100 == 1)
                    Debug.Log($"[Chuva {_numObjetos}] coleta sim_t={_stepMs.Count * 0.02:F1}s step={stepMs:F2}ms n={_stepMs.Count}");
                if (_stepMs.Count >= WINDOW_STEPS)
                    Finalizar();
            }
        }

        void Finalizar()
        {
            _running = false;
            _coletando = false;

            float stepMedio = Media(_stepMs);
            float stepMax = Maximo(_stepMs);
            float fpsMedio = Media(_fps);
            int amostras = _stepMs.Count;

            BenchmarkCommon.AppendLine(_csvPath, string.Format(
                System.Globalization.CultureInfo.InvariantCulture,
                "{0},{1},{2:F4},{3:F4},{4:F2},{5}",
                _run + 1, _numObjetos, stepMedio, stepMax, fpsMedio, amostras));

            Debug.Log($"[Chuva {_numObjetos}] Run {_run + 1}/{_totalRuns} step={stepMedio:F3}ms " +
                      $"(max {stepMax:F3}) fps={fpsMedio:F1} amostras={amostras}");

            StartCoroutine(CiclarProximaRun());
        }

        IEnumerator CiclarProximaRun()
        {
            // Reset in-place (aprendido em Godot/Jolt: scene reload em 10k crasha
            // na 2a run). Destroi esferas, espera o PhysX limpar, respawn.
            foreach (var s in _spheres)
                if (s != null) Destroy(s.gameObject);
            _spheres.Clear();

            yield return new WaitForFixedUpdate();
            yield return new WaitForFixedUpdate();
            yield return new WaitForSeconds(0.3f);

            _run++;
            if (_run >= _totalRuns)
            {
                _run = 0;
                _varIdx++;
                if (_varIdx >= _variacoes.Length)
                {
                    Finalizado();
                    yield break;
                }
                Debug.Log($"[Chuva] Avancando para variacao={_variacoes[_varIdx]}");
            }
            IniciarProximaRun();
        }

        void Finalizado()
        {
            Debug.Log($"[Chuva] CONCLUIDO. Resultados em {_csvPath}");
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
            Application.Quit();
#endif
        }

        static float Media(List<float> a)
        {
            if (a.Count == 0) return 0f;
            float s = 0f; foreach (var v in a) s += v; return s / a.Count;
        }

        static float Maximo(List<float> a)
        {
            if (a.Count == 0) return 0f;
            float m = a[0]; foreach (var v in a) if (v > m) m = v; return m;
        }
    }
}
