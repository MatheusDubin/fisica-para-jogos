using UnityEngine;

namespace Benchmark
{
    // Ponto de entrada totalmente automatico. Assim que a cena carrega (qualquer
    // cena), cria o GameObject do benchmark e dispara o cenario resolvido. Nao
    // exige nenhum setup manual na cena nem ligacao de Inspector - basta dar Play
    // (ou rodar o build), exatamente como o autoload do Godot.
    public static class BenchmarkBootstrap
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        private static void Boot()
        {
            BenchmarkCommon.ApplyGlobalSettings();

            var scenario = BenchmarkCommon.ResolveScenario();
            var go = new GameObject("[Benchmark]");
            Object.DontDestroyOnLoad(go);

            switch (scenario)
            {
                case Scenario.Rain:
                    go.AddComponent<RainBenchmark>();
                    break;
                default:
                    go.AddComponent<TowerBenchmark>();
                    break;
            }

            Debug.Log($"[Benchmark] Bootstrap -> cenario={scenario}, saida={BenchmarkCommon.ResolveOutputRoot()}");
        }
    }
}
