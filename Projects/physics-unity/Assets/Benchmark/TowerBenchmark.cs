using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Benchmark
{
    // Cenario 1 - A Torre
    // -------------------------------------------------------------------------
    // 1. Constroi uma arena fechada (chao 60x60 + 4 paredes + teto, altura 150m).
    // 2. Empilha N cubos perfeitamente (faces tocando, spacing 1.0).
    // 3. A cada FixedUpdate conta quantos Rigidbody estao dormindo (IsSleeping).
    // 4. Quando todos dormem (ou timeout 60s) registra tempo + metricas e faz
    //    reset in-place (destroi cubos, espera 2 FixedUpdates, respawn).
    // 5. Repete RUNS_POR_N vezes para cada N da lista N_VALUES e encerra.
    //
    // A lista de N e o numero de runs sao configuraveis (sweep de stress-test):
    //   - PlayerPrefs "bench_tower_n" / "bench_tower_runs" / "bench_tower_subdir"
    //     (setados pelo menu do Editor), ou
    //   - env BENCH_TOWER_N / BENCH_TOWER_RUNS / BENCH_TOWER_SUBDIR (headless).
    //   - Default: N=100, 10 runs, pasta torre-N100-arena (entrega canonica).
    //
    // Decisoes travadas cross-engine (ver scenario-1-torre-unity.md):
    //  - spacing 1.0 exato (sem gap), bounce 0.0, arena fechada 60x60x150.
    //  - spawn 100% em codigo, sleep check em FixedUpdate, recursos compartilhados.
    //
    // O tempo ate sleep e medido em TEMPO DE SIMULACAO (n_frames * fixedDeltaTime),
    // nao wall-clock: e deterministico e e a leitura fisicamente correta do
    // "tempo ate a pilha parar". phys_frames tambem e gravado para auditoria.
    public class TowerBenchmark : MonoBehaviour
    {
        const float CUBE_SIDE = 1.0f;
        const float SPACING = 1.0f;        // faces tocando, sem gap ("perfeitamente empilhadas")
        const float OFFSET_INICIAL = 0.5f; // base do cubo mais baixo no topo do chao (y=0)
        const float TIMEOUT_S = 60.0f;
        const bool ENABLE_DEBUG = true;

        const string CSV = "torre_unity.csv";
        const string DEBUG_CSV = "torre_unity_debug.csv";
        const string CSV_HEADER = "run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep";
        const string DEBUG_HEADER = "run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y";

        // Configuracao resolvida em Start().
        int[] _nValues = { 100 };
        int _runsPorN = 10;
        string _subdir = "torre-N100-arena";
        // Iteracoes do solver por Rigidbody. 0 = padrao da engine (nao mexer) =
        // variante DEFAULT. >0 = variante TUNED (ex: 20) para demonstrar que com
        // mais iteracoes a pilha se sustenta. Aplicado identico nas 3 engines.
        int _solverIter = 0;

        readonly List<Rigidbody> _corpos = new List<Rigidbody>();
        PhysicsMaterial _sharedMat;
        GameObject _viewer;
        GameObject _cubesRoot;

        int _nIdx = 0;             // indice na lista de N
        int _run = 0;              // 0-based dentro do N atual
        bool _running = false;

        float _simElapsed = 0f;
        int _physFrames = 0;
        float _maxV = 0f;
        float _tFirstSleep = -1f;
        float _tHalfSleep = -1f;
        readonly List<string> _debugBuf = new List<string>();

        string _csvPath, _debugPath;

        int N => _nValues[_nIdx];

        void Start()
        {
            ResolverConfig();

            _csvPath = BenchmarkCommon.PrepareFile(_subdir, CSV);
            _debugPath = BenchmarkCommon.PrepareFile(_subdir, DEBUG_CSV);
            BenchmarkCommon.WriteHeader(_csvPath, CSV_HEADER);
            if (ENABLE_DEBUG) BenchmarkCommon.WriteHeader(_debugPath, DEBUG_HEADER);

            _sharedMat = CriarMaterial();
            CriarArena();

            string modo = _solverIter > 0 ? $"TUNED(solverIter={_solverIter})" : $"DEFAULT(solverIter={Physics.defaultSolverIterations})";
            Debug.Log($"[Torre] Unity {Application.unityVersion}, modo={modo}, " +
                      $"sleepThreshold={Physics.sleepThreshold}, fixedDelta={Time.fixedDeltaTime}, " +
                      $"N=[{string.Join(",", _nValues)}], runs/N={_runsPorN}, saida={_subdir}");

            StartRun();
        }

        void ResolverConfig()
        {
            _nValues = ParseIntList(GetCfg("BENCH_TOWER_N", "bench_tower_n", "100")) ?? new[] { 100 };
            if (int.TryParse(GetCfg("BENCH_TOWER_RUNS", "bench_tower_runs", "10"), out int r) && r > 0)
                _runsPorN = r;
            _subdir = GetCfg("BENCH_TOWER_SUBDIR", "bench_tower_subdir", "torre-N100-arena");
            if (int.TryParse(GetCfg("BENCH_TOWER_SOLVERITER", "bench_tower_solveriter", "0"), out int si) && si >= 0)
                _solverIter = si;
        }

        static string GetCfg(string envKey, string prefKey, string def)
        {
            string env = Environment.GetEnvironmentVariable(envKey);
            if (!string.IsNullOrEmpty(env)) return env;
            return PlayerPrefs.GetString(prefKey, def);
        }

        static int[] ParseIntList(string raw)
        {
            if (string.IsNullOrEmpty(raw)) return null;
            var list = new List<int>();
            foreach (var p in raw.Split(','))
                if (int.TryParse(p.Trim(), out int v) && v > 0) list.Add(v);
            return list.Count > 0 ? list.ToArray() : null;
        }

        PhysicsMaterial CriarMaterial()
        {
            return new PhysicsMaterial("torre")
            {
                dynamicFriction = 0.4f,
                staticFriction = 0.5f,
                bounciness = 0.0f, // critico para a Torre
                frictionCombine = PhysicsMaterialCombine.Average,
                bounceCombine = PhysicsMaterialCombine.Minimum,
            };
        }

        // Arena fechada, dimensionada para o maior N do sweep. Paredes a 30m do
        // centro: nao influenciam a fase inicial (cubos so as tocam se o solver
        // ejetar). Contem cubos ejetados para a metrica continuar valida.
        void CriarArena()
        {
            int maxN = 1;
            foreach (var n in _nValues) if (n > maxN) maxN = n;

            const float largura = 60f, profundidade = 60f, esp = 2f;
            float altura = Mathf.Max(150f, maxN * SPACING * 1.5f);
            float meioY = altura * 0.5f;
            var root = new GameObject("Arena");

            // Apenas o chao e visivel; paredes e teto sao collision-only (igual ao
            // Godot), para a camera externa ver os cubos e o FPS nao incluir o
            // custo de renderizar paredes que o Godot nao renderiza.
            CriarParede(root, new Vector3(0, -esp * 0.5f, 0), new Vector3(largura, esp, profundidade), true);
            CriarParede(root, new Vector3(0, altura + esp * 0.5f, 0), new Vector3(largura, esp, profundidade), false);
            CriarParede(root, new Vector3(largura * 0.5f + esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade), false);
            CriarParede(root, new Vector3(-largura * 0.5f - esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade), false);
            CriarParede(root, new Vector3(0, meioY, profundidade * 0.5f + esp * 0.5f), new Vector3(largura, altura, esp), false);
            CriarParede(root, new Vector3(0, meioY, -profundidade * 0.5f - esp * 0.5f), new Vector3(largura, altura, esp), false);
        }

        void CriarParede(GameObject parent, Vector3 pos, Vector3 size, bool visible)
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

        // Reposiciona a camera para enquadrar uma torre de N cubos.
        void EnquadrarCamera()
        {
            if (_viewer != null) Destroy(_viewer);
            float altura = N * SPACING;
            float dist = Mathf.Max(20f, altura * 1.2f);
            _viewer = BenchmarkCommon.CreateViewer(
                "TorreViewer",
                new Vector3(dist, altura * 0.5f, dist),
                new Vector3(0f, altura * 0.25f, 0f),
                4000f);
        }

        void StartRun()
        {
            _simElapsed = 0f;
            _physFrames = 0;
            _maxV = 0f;
            _tFirstSleep = -1f;
            _tHalfSleep = -1f;
            _debugBuf.Clear();

            EnquadrarCamera();

            _cubesRoot = new GameObject($"Cubos_N{N}_run{_run + 1}");
            for (int i = 0; i < N; i++)
            {
                float y = OFFSET_INICIAL + i * SPACING;
                var rb = CriarCubo(new Vector3(0, y, 0));
                _corpos.Add(rb);
            }

            _running = true;
            Debug.Log($"[Torre] N={N} Run {_run + 1}/{_runsPorN} iniciada");
        }

        Rigidbody CriarCubo(Vector3 pos)
        {
            var go = GameObject.CreatePrimitive(PrimitiveType.Cube);
            go.transform.SetParent(_cubesRoot.transform);
            go.transform.position = pos;
            go.transform.localScale = Vector3.one * CUBE_SIDE;
            go.GetComponent<Collider>().sharedMaterial = _sharedMat;

            var rb = go.AddComponent<Rigidbody>();
            rb.mass = 1.0f;
            rb.collisionDetectionMode = CollisionDetectionMode.Discrete; // padrao (spec)
            if (_solverIter > 0)
            {
                // Variante TUNED: mais iteracoes do solver propagam a forca de
                // suporte do chao ate o topo dentro de um passo -> pilha estavel.
                rb.solverIterations = _solverIter;
                rb.solverVelocityIterations = _solverIter;
            }
            return rb;
        }

        void FixedUpdate()
        {
            if (!_running) return;

            _physFrames++;
            _simElapsed += Time.fixedDeltaTime;

            int dormindo = 0;
            float maxV = 0f, sumV = 0f, topY = float.NegativeInfinity;
            for (int i = 0; i < _corpos.Count; i++)
            {
                var c = _corpos[i];
                if (c.IsSleeping()) dormindo++;
                float v = c.linearVelocity.magnitude;
                if (v > maxV) maxV = v;
                sumV += v;
                float y = c.transform.position.y;
                if (y > topY) topY = y;
            }
            float meanV = _corpos.Count > 0 ? sumV / _corpos.Count : 0f;

            if (maxV > _maxV) _maxV = maxV;
            if (dormindo > 0 && _tFirstSleep < 0f) _tFirstSleep = _simElapsed;
            if (dormindo >= _corpos.Count / 2 && _tHalfSleep < 0f) _tHalfSleep = _simElapsed;

            if (ENABLE_DEBUG)
            {
                int activeObjs = _corpos.Count - dormindo;
                _debugBuf.Add(string.Format(System.Globalization.CultureInfo.InvariantCulture,
                    "{0},{1},{2:F4},{3},{4},{5},{6:F6},{7:F6},{8:F4}",
                    _run + 1, N, _simElapsed, _physFrames,
                    dormindo, activeObjs, maxV, meanV, topY));
            }

            bool todosDormindo = dormindo == _corpos.Count;
            bool timeout = _simElapsed >= TIMEOUT_S;
            if (todosDormindo || timeout)
            {
                _running = false;
                RegistrarRun(timeout, dormindo);
                StartCoroutine(ProximaRun());
            }
        }

        void RegistrarRun(bool timeout, int dormindo)
        {
            BenchmarkCommon.AppendLine(_csvPath, string.Format(
                System.Globalization.CultureInfo.InvariantCulture,
                "{0},{1},{2:F4},{3},{4},{5:F6},{6:F4},{7:F4}",
                _run + 1, N, _simElapsed, timeout ? 1 : 0,
                _physFrames, _maxV, _tFirstSleep, _tHalfSleep));

            if (ENABLE_DEBUG)
                foreach (var l in _debugBuf) BenchmarkCommon.AppendLine(_debugPath, l);

            Debug.Log($"[Torre] N={N} Run {_run + 1}/{_runsPorN} -> {_simElapsed:F3}s " +
                      $"(frames={_physFrames}, max_v={_maxV:F2}, dormindo={dormindo}/{N})" +
                      (timeout ? " (TIMEOUT)" : ""));
        }

        IEnumerator ProximaRun()
        {
            yield return new WaitForSeconds(0.2f); // flush do CSV

            foreach (var c in _corpos)
                if (c != null) Destroy(c.gameObject);
            _corpos.Clear();
            if (_cubesRoot != null) Destroy(_cubesRoot);

            // Espera o PhysX limpar os bodies destruidos antes do proximo spawn.
            yield return new WaitForFixedUpdate();
            yield return new WaitForFixedUpdate();

            _run++;
            if (_run >= _runsPorN)
            {
                _run = 0;
                _nIdx++;
                if (_nIdx >= _nValues.Length)
                {
                    Finalizar();
                    yield break;
                }
                Debug.Log($"[Torre] Avancando para N={N}");
            }
            StartRun();
        }

        void Finalizar()
        {
            Debug.Log($"[Torre] CONCLUIDO. Resultados em {_csvPath}");
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
            Application.Quit();
#endif
        }
    }
}
