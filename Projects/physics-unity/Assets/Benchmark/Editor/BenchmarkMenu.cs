using UnityEditor;
using UnityEngine;

namespace Benchmark.EditorTools
{
    // Menu do Editor para rodar cada cenario com um clique. Seta o PlayerPref
    // lido pelo BenchmarkBootstrap e entra em Play. Nenhuma ligacao de Inspector
    // necessaria - o cenario inteiro e gerado em codigo.
    public static class BenchmarkMenu
    {
        [MenuItem("Benchmark/Rodar Torre (10 runs)")]
        public static void RunTower()
        {
            PlayerPrefs.SetString("bench_scenario", "tower");
            PlayerPrefs.Save();
            EnterPlay();
        }

        [MenuItem("Benchmark/Rodar Chuva (1k 5k 10k)")]
        public static void RunRain()
        {
            PlayerPrefs.SetString("bench_scenario", "rain");
            PlayerPrefs.Save();
            EnterPlay();
        }

        [MenuItem("Benchmark/Abrir pasta de resultados")]
        public static void OpenResults()
        {
            string dir = BenchmarkCommon.ResolveOutputRoot();
            System.IO.Directory.CreateDirectory(dir);
            EditorUtility.RevealInFinder(dir);
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
