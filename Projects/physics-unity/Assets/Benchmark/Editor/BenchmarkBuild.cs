using System;
using UnityEditor;
using UnityEditor.Build.Reporting;
using UnityEngine;

namespace Benchmark.EditorTools
{
    // Build headless de um player standalone, chamado via:
    //   Unity.exe -batchmode -quit -projectPath <proj> -executeMethod Benchmark.EditorTools.BenchmarkBuild.Build
    //
    // Controle por variavel de ambiente:
    //   BENCH_BUILD_PATH  caminho do .exe de saida (default: Build/Benchmark.exe)
    //   BENCH_DEV=1       habilita Development Build (profiler). Default: Release.
    //
    // O cenario e a pasta de saida dos CSVs sao escolhidos em tempo de execucao
    // do player via BENCH_SCENARIO / BENCH_OUT (ver BenchmarkCommon).
    public static class BenchmarkBuild
    {
        public static void Build()
        {
            string outPath = Environment.GetEnvironmentVariable("BENCH_BUILD_PATH");
            if (string.IsNullOrEmpty(outPath))
                outPath = System.IO.Path.Combine(
                    System.IO.Path.GetDirectoryName(Application.dataPath), "Build", "Benchmark.exe");

            bool dev = Environment.GetEnvironmentVariable("BENCH_DEV") == "1";

            var opts = new BuildPlayerOptions
            {
                scenes = new[] { "Assets/Scenes/SampleScene.unity" },
                locationPathName = outPath,
                target = BuildTarget.StandaloneWindows64,
                options = dev ? (BuildOptions.Development) : BuildOptions.None,
            };

            Debug.Log($"[BenchmarkBuild] Build -> {outPath} (dev={dev})");
            BuildReport report = BuildPipeline.BuildPlayer(opts);
            BuildSummary summary = report.summary;

            if (summary.result == BuildResult.Succeeded)
                Debug.Log($"[BenchmarkBuild] OK: {summary.totalSize} bytes em {summary.outputPath}");
            else
            {
                Debug.LogError($"[BenchmarkBuild] FALHOU: {summary.result}");
                EditorApplication.Exit(1);
            }
        }
    }
}
