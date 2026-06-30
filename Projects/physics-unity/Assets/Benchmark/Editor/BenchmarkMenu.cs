using UnityEditor;
using UnityEngine;

namespace Benchmark.EditorTools
{
    // Menu do Editor. Os itens NUMERADOS (1..5) sao a ordem de coleta do dataset
    // final (30 ciclos cada). Os "Archived -" sao exploratorios ja analisados -
    // NAO precisam ser rodados de novo (os dados deles ja estao no repo).
    //
    // ATENCAO ao Solver Type (Project Settings > Physics > Solver Type): nao ha
    // API de runtime para troca-lo. Itens 1-3 rodam em PGS (default); itens 4-5
    // exigem trocar para TGS antes. Depois do 5, volte para PGS.
    //
    // Tudo e gerado em codigo - nenhuma ligacao de Inspector necessaria.
    public static class BenchmarkMenu
    {
        const string RUNS = "30";       // ciclos do dataset final
        const int DEFAULT_ITER = 0;     // 0 = padrao da engine (6)
        const int TUNED_ITER = 20;      // variante exploratoria +iter

        // =================================================================
        //  DATASET FINAL - rodar nesta ordem (30 ciclos cada)
        // =================================================================

        // ---- Fase A: Solver Type = Projected Gauss Seidel (PGS, default) ----

        [MenuItem("Benchmark/1. Torre N=100 PGS default (30 runs)", priority = 0)]
        public static void Run1() =>
            SetTower("100", RUNS, "torre-N100-arena", DEFAULT_ITER);

        [MenuItem("Benchmark/2. Torre SWEEP PGS 10..30 (+5, 30 runs)", priority = 1)]
        public static void Run2() =>
            SetTower("10,15,20,25,30", RUNS, "torre-sweep-default", DEFAULT_ITER);

        [MenuItem("Benchmark/3. Chuva 1k 5k 10k (30 runs)", priority = 2)]
        public static void Run3()
        {
            PlayerPrefs.SetString("bench_scenario", "rain");
            PlayerPrefs.SetString("bench_rain_runs", RUNS);
            PlayerPrefs.Save();
            EnterPlay();
        }

        // ---- Fase B: trocar Solver Type para Temporal Gauss Seidel (TGS) ----

        [MenuItem("Benchmark/4. Torre N=100 TGS  [trocar p/ TGS antes] (30 runs)", priority = 3)]
        public static void Run4() =>
            SetTower("100", RUNS, "torre-N100-tgs", DEFAULT_ITER);

        [MenuItem("Benchmark/5. Torre SWEEP TGS 10..30  [trocar p/ TGS antes] (30 runs)", priority = 4)]
        public static void Run5() =>
            SetTower("10,15,20,25,30", RUNS, "torre-sweep-tgs", DEFAULT_ITER);

        // =================================================================
        //  Utilidades
        // =================================================================

        [MenuItem("Benchmark/Abrir pasta de resultados", priority = 40)]
        public static void OpenResults()
        {
            string dir = BenchmarkCommon.ResolveOutputRoot();
            System.IO.Directory.CreateDirectory(dir);
            EditorUtility.RevealInFinder(dir);
        }

        // =================================================================
        //  ARQUIVADOS - exploratorios ja analisados, NAO rodar de novo
        // =================================================================

        [MenuItem("Benchmark/Archived - Torre N=100 +iter PGS 20 (10 runs)", priority = 100)]
        public static void ArchivedTowerIter() =>
            SetTower("100", "10", "torre-N100-tuned", TUNED_ITER);

        [MenuItem("Benchmark/Archived - Torre SWEEP +iter PGS 10..100 (3 runs)", priority = 101)]
        public static void ArchivedSweepIter() =>
            SetTower("10,20,30,40,50,60,70,80,90,100", "3", "torre-sweep-tuned", TUNED_ITER);

        // =================================================================

        static void SetTower(string n, string runs, string subdir, int solverIter)
        {
            PlayerPrefs.SetString("bench_scenario", "tower");
            PlayerPrefs.SetString("bench_tower_n", n);
            PlayerPrefs.SetString("bench_tower_runs", runs);
            PlayerPrefs.SetString("bench_tower_subdir", subdir);
            PlayerPrefs.SetString("bench_tower_solveriter", solverIter.ToString());
            PlayerPrefs.Save();
            EnterPlay();
        }

        static void EnterPlay()
        {
            if (EditorApplication.isPlaying)
            {
                Debug.LogWarning("[Benchmark] Ja esta em Play. Pare e rode de novo.");
                return;
            }
            EditorApplication.EnterPlaymode();
        }
    }
}
