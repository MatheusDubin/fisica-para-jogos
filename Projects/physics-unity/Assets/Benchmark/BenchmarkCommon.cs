using System;
using System.IO;
using UnityEngine;

namespace Benchmark
{
    // Cenarios suportados. Sempre rodamos UM cenario por sessao - Torre e Chuva
    // tem tempos de execucao e modos de falha diferentes (a Chuva em 10k pode
    // ate precisar de tratamento de crash), entao misturar os dois numa unica
    // sessao so adiciona risco sem beneficio.
    public enum Scenario { Tower, Rain }

    // Utilidades compartilhadas pelos dois cenarios: resolucao do diretorio de
    // saida, escrita de CSV e aplicacao das configuracoes globais exigidas pelo
    // spec (V-Sync off, frame cap removido).
    //
    // Filosofia: TUDO em codigo. Nenhuma referencia de cena ou de Inspector
    // precisa ser ligada a mao - espelha a implementacao Godot, que gera a cena
    // inteira via script para garantir reprodutibilidade.
    public static class BenchmarkCommon
    {
        // Resolve qual cenario rodar, em ordem de prioridade:
        //   1. variavel de ambiente BENCH_SCENARIO (tower|rain) - usado no
        //      build headless via linha de comando.
        //   2. PlayerPrefs "bench_scenario" - setado pelo menu do Editor.
        //   3. padrao: Tower.
        public static Scenario ResolveScenario()
        {
            string raw = Environment.GetEnvironmentVariable("BENCH_SCENARIO");
            if (string.IsNullOrEmpty(raw))
                raw = PlayerPrefs.GetString("bench_scenario", "tower");
            switch (raw.Trim().ToLowerInvariant())
            {
                case "rain": case "chuva": return Scenario.Rain;
                default: return Scenario.Tower;
            }
        }

        // Diretorio raiz onde os CSVs sao gravados. Prioridade:
        //   1. variavel de ambiente BENCH_OUT (absoluto) - usado no build headless
        //      para apontar direto para assignment-b/results/unity.
        //   2. No Editor: caminho relativo ao repo (.../assignment-b/results/unity).
        //   3. Fallback: Application.persistentDataPath.
        public static string ResolveOutputRoot()
        {
            string env = Environment.GetEnvironmentVariable("BENCH_OUT");
            if (!string.IsNullOrEmpty(env))
                return env;

            if (Application.isEditor)
            {
                // Application.dataPath = .../Projects/physics-unity/Assets
                // tres niveis acima = .../fisica-para-jogos
                string repo = Path.GetFullPath(Path.Combine(Application.dataPath, "../../.."));
                return Path.Combine(repo, "assignment-b", "results", "unity");
            }

            return Application.persistentDataPath;
        }

        // Aplica as configuracoes globais do spec. Chamado uma vez no bootstrap.
        public static void ApplyGlobalSettings()
        {
            QualitySettings.vSyncCount = 0;     // V-Sync off (spec)
            Application.targetFrameRate = -1;   // sem frame cap (spec)
        }

        // Garante que o diretorio existe e retorna o caminho completo do arquivo.
        public static string PrepareFile(string subDir, string fileName)
        {
            string dir = Path.Combine(ResolveOutputRoot(), subDir);
            Directory.CreateDirectory(dir);
            return Path.Combine(dir, fileName);
        }

        // Trunca o arquivo e escreve o cabecalho (primeira run da sessao).
        public static void WriteHeader(string path, string header)
        {
            File.WriteAllText(path, header + "\n");
        }

        // Adiciona uma linha ao final do arquivo.
        public static void AppendLine(string path, string line)
        {
            File.AppendAllText(path, line + "\n");
        }

        // Cria uma camera + luz direcional simples e posiciona a camera olhando
        // para um alvo. Retorna o GameObject raiz para poder destruir depois.
        public static GameObject CreateViewer(string name, Vector3 camPos, Vector3 lookAt, float far)
        {
            var root = new GameObject(name);

            var camGo = new GameObject("Camera");
            camGo.transform.SetParent(root.transform);
            var cam = camGo.AddComponent<Camera>();
            cam.farClipPlane = far;
            camGo.transform.position = camPos;
            camGo.transform.LookAt(lookAt, Vector3.up);

            var lightGo = new GameObject("Sun");
            lightGo.transform.SetParent(root.transform);
            var light = lightGo.AddComponent<Light>();
            light.type = LightType.Directional;
            light.intensity = 1.0f;
            lightGo.transform.rotation = Quaternion.Euler(50f, -35f, 0f);

            return root;
        }
    }
}
