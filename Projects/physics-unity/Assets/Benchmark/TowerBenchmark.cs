using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Benchmark
{
    // Cenario 1 - A Torre
    // -------------------------------------------------------------------------
    // 1. Constroi uma arena fechada (chao 60x60 + 4 paredes + teto, altura 150m).
    // 2. Empilha NUM_CUBOS cubos perfeitamente (faces tocando, spacing 1.0).
    // 3. A cada FixedUpdate conta quantos Rigidbody estao dormindo (IsSleeping).
    // 4. Quando todos dormem (ou timeout 60s) registra tempo + metricas e faz
    //    reset in-place (destroi cubos, espera 2 FixedUpdates, respawn).
    // 5. Repete por TOTAL_RUNS (10) e encerra.
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
        const int NUM_CUBOS = 100;
        const float CUBE_SIDE = 1.0f;
        const float SPACING = 1.0f;       // faces tocando, sem gap ("perfeitamente empilhadas")
        const float OFFSET_INICIAL = 0.5f; // base do cubo mais baixo no topo do chao (y=0)
        const int TOTAL_RUNS = 10;
        const float TIMEOUT_S = 60.0f;
        const bool ENABLE_DEBUG = true;

        const string SUBDIR = "torre-N100-arena";
        const string CSV = "torre_unity.csv";
        const string DEBUG_CSV = "torre_unity_debug.csv";
        const string CSV_HEADER = "run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep";
        const string DEBUG_HEADER = "run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y";

        readonly List<Rigidbody> _corpos = new List<Rigidbody>();
        PhysicsMaterial _sharedMat;
        GameObject _viewer;
        GameObject _cubesRoot;

        int _run = 0;              // 0-based
        bool _running = false;

        float _simElapsed = 0f;
        int _physFrames = 0;
        float _maxV = 0f;
        float _tFirstSleep = -1f;
        float _tHalfSleep = -1f;
        readonly List<string> _debugBuf = new List<string>();

        string _csvPath, _debugPath;

        void Start()
        {
            _csvPath = BenchmarkCommon.PrepareFile(SUBDIR, CSV);
            _debugPath = BenchmarkCommon.PrepareFile(SUBDIR, DEBUG_CSV);
            BenchmarkCommon.WriteHeader(_csvPath, CSV_HEADER);
            if (ENABLE_DEBUG) BenchmarkCommon.WriteHeader(_debugPath, DEBUG_HEADER);

            _sharedMat = CriarMaterial();
            CriarArena();

            float altura = NUM_CUBOS * SPACING;
            float dist = Mathf.Max(20f, altura * 1.2f);
            _viewer = BenchmarkCommon.CreateViewer(
                "TorreViewer",
                new Vector3(dist, altura * 0.5f, dist),
                new Vector3(0f, altura * 0.25f, 0f),
                4000f);

            Debug.Log($"[Torre] Unity {Application.unityVersion}, solverIter={Physics.defaultSolverIterations}, " +
                      $"sleepThreshold={Physics.sleepThreshold}, fixedDelta={Time.fixedDeltaTime}");

            StartRun();
        }

        PhysicsMaterial CriarMaterial()
        {
            var m = new PhysicsMaterial("torre")
            {
                dynamicFriction = 0.4f,
                staticFriction = 0.5f,
                bounciness = 0.0f, // critico para a Torre
                frictionCombine = PhysicsMaterialCombine.Average,
                bounceCombine = PhysicsMaterialCombine.Minimum,
            };
            return m;
        }

        // Arena fechada. Paredes a 30m do centro: nao influenciam a fase inicial
        // (cubos so as tocam se o solver ejetar). Contem cubos ejetados para que
        // a metrica de tempo-ate-sleep continue valida mesmo em colapso.
        void CriarArena()
        {
            const float largura = 60f, profundidade = 60f, esp = 2f;
            float altura = Mathf.Max(150f, NUM_CUBOS * SPACING * 1.5f);
            float meioY = altura * 0.5f;
            var root = new GameObject("Arena");

            // Chao
            CriarParede(root, new Vector3(0, -esp * 0.5f, 0), new Vector3(largura, esp, profundidade));
            // Teto
            CriarParede(root, new Vector3(0, altura + esp * 0.5f, 0), new Vector3(largura, esp, profundidade));
            // Paredes eixo X
            CriarParede(root, new Vector3(largura * 0.5f + esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade));
            CriarParede(root, new Vector3(-largura * 0.5f - esp * 0.5f, meioY, 0), new Vector3(esp, altura, profundidade));
            // Paredes eixo Z
            CriarParede(root, new Vector3(0, meioY, profundidade * 0.5f + esp * 0.5f), new Vector3(largura, altura, esp));
            CriarParede(root, new Vector3(0, meioY, -profundidade * 0.5f - esp * 0.5f), new Vector3(largura, altura, esp));
        }

        void CriarParede(GameObject parent, Vector3 pos, Vector3 size)
        {
            var go = GameObject.CreatePrimitive(PrimitiveType.Cube);
            go.name = "Parede";
            go.transform.SetParent(parent.transform);
            go.transform.position = pos;
            go.transform.localScale = size;
            // Static body: collider sem Rigidbody. Mesmo material dos cubos.
            go.GetComponent<Collider>().sharedMaterial = _sharedMat;
            go.isStatic = true;
        }

        void StartRun()
        {
            _simElapsed = 0f;
            _physFrames = 0;
            _maxV = 0f;
            _tFirstSleep = -1f;
            _tHalfSleep = -1f;
            _debugBuf.Clear();

            _cubesRoot = new GameObject($"Cubos_run{_run + 1}");
            for (int i = 0; i < NUM_CUBOS; i++)
            {
                float y = OFFSET_INICIAL + i * SPACING;
                var rb = CriarCubo(new Vector3(0, y, 0));
                _corpos.Add(rb);
            }

            _running = true;
            Debug.Log($"[Torre] Run {_run + 1}/{TOTAL_RUNS} iniciada (cubos={NUM_CUBOS})");
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
                    _run + 1, NUM_CUBOS, _simElapsed, _physFrames,
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
                _run + 1, NUM_CUBOS, _simElapsed, timeout ? 1 : 0,
                _physFrames, _maxV, _tFirstSleep, _tHalfSleep));

            if (ENABLE_DEBUG)
                foreach (var l in _debugBuf) BenchmarkCommon.AppendLine(_debugPath, l);

            Debug.Log($"[Torre] Run {_run + 1}/{TOTAL_RUNS} -> {_simElapsed:F3}s " +
                      $"(frames={_physFrames}, max_v={_maxV:F2}, dormindo={dormindo}/{NUM_CUBOS})" +
                      (timeout ? " (TIMEOUT)" : ""));
        }

        IEnumerator ProximaRun()
        {
            // Pequeno buffer para garantir flush do CSV.
            yield return new WaitForSeconds(0.2f);

            // Reset in-place: destroi os cubos, mantem arena/camera/luz.
            foreach (var c in _corpos)
                if (c != null) Destroy(c.gameObject);
            _corpos.Clear();
            if (_cubesRoot != null) Destroy(_cubesRoot);

            // Espera o PhysX limpar os bodies destruidos antes do proximo spawn.
            yield return new WaitForFixedUpdate();
            yield return new WaitForFixedUpdate();

            _run++;
            if (_run < TOTAL_RUNS)
                StartRun();
            else
                Finalizar();
        }

        void Finalizar()
        {
            Debug.Log($"[Torre] CONCLUIDO. {TOTAL_RUNS} runs gravadas em {_csvPath}");
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
            Application.Quit();
#endif
        }
    }
}
