#!/usr/bin/env python3
"""
Agrega os CSVs de resultado das 3 engines e aplica a metodologia estatistica
EXATA do enunciado do trabalho (Grau B):

  1. N execucoes isoladas -> valor por run.
  2. Media simples (M) e variancia da amostra -> desvio padrao (sigma).
     (variancia populacional: soma((v-M)^2)/n, i.e. STDEVP, conforme howto-statistics.md)
  3. Descartar runs fora de [M - sigma, M + sigma].
  4. Media Final apenas com os que sobraram.

Le o formato REAL dos nossos CSVs (nao o formato hipotetico do howto):
  torre_<engine>.csv : run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,...
  chuva_<engine>.csv : run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras

Uso:
  python aggregate_stats.py            # varre results/{godot,unity,unreal}/**, gera RESULTS.md
  python aggregate_stats.py --print    # so imprime no console, nao escreve arquivo
"""

import csv
import glob
import math
import os
import sys

RESULTS_DIR = os.path.dirname(os.path.abspath(__file__))
ENGINES = ["godot", "unity", "unreal"]


def stats(values):
    """Aplica os 4 passos. Retorna dict com M, sigma, faixa, descartados, media_final."""
    n = len(values)
    if n == 0:
        return None
    m = sum(values) / n
    var = sum((v - m) ** 2 for v in values) / n          # populacional (/n) = STDEVP
    sigma = math.sqrt(var)
    lo, hi = m - sigma, m + sigma
    kept = [v for v in values if lo - 1e-9 <= v <= hi + 1e-9]
    dropped = [v for v in values if not (lo - 1e-9 <= v <= hi + 1e-9)]
    final = (sum(kept) / len(kept)) if kept else m
    return dict(n=n, mean=m, var=var, sigma=sigma, lo=lo, hi=hi,
                kept=kept, dropped=dropped, final=final)


def read_csv_rows(path):
    with open(path, newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def collect_torre():
    """Retorna lista de (engine, config, N, [tempos])."""
    out = []
    for eng in ENGINES:
        for path in sorted(glob.glob(os.path.join(RESULTS_DIR, eng, "**", "torre_*.csv"), recursive=True)):
            if "debug" in os.path.basename(path):
                continue
            config = os.path.basename(os.path.dirname(path))
            rows = read_csv_rows(path)
            by_n = {}
            for r in rows:
                try:
                    n = int(float(r["num_cubos"]))
                    t = float(r["tempo_ate_sleep_s"])
                except (KeyError, ValueError):
                    continue
                by_n.setdefault(n, []).append(t)
            for n in sorted(by_n):
                out.append((eng, config, n, by_n[n]))
    return out


def collect_chuva():
    """Retorna lista de (engine, config, variacao, [step_ms], [fps])."""
    out = []
    for eng in ENGINES:
        for path in sorted(glob.glob(os.path.join(RESULTS_DIR, eng, "**", "chuva_*.csv"), recursive=True)):
            config = os.path.basename(os.path.dirname(path))
            rows = read_csv_rows(path)
            by_var = {}
            for r in rows:
                try:
                    v = int(float(r["variacao"]))
                    step = float(r["physics_step_ms_medio"])
                    fps = float(r.get("fps_medio", "nan"))
                except (KeyError, ValueError):
                    continue
                d = by_var.setdefault(v, {"step": [], "fps": []})
                d["step"].append(step)
                if not math.isnan(fps):
                    d["fps"].append(fps)
            for v in sorted(by_var):
                out.append((eng, config, v, by_var[v]["step"], by_var[v]["fps"]))
    return out


def collect_torre_stability():
    """Do debug CSV: por (engine,config,N) calcula top_y inicial/final e classifica
    ESTAVEL / PARCIAL / COLAPSOU (colapso = a pilha achatou para o chao)."""
    out = []
    for eng in ENGINES:
        for path in sorted(glob.glob(os.path.join(RESULTS_DIR, eng, "**", "torre_*debug*.csv"), recursive=True)):
            config = os.path.basename(os.path.dirname(path))
            g = {}  # (N,run) -> [first, last, min] top_y
            for r in read_csv_rows(path):
                try:
                    N = int(float(r["num_cubos"])); run = int(float(r["run"])); ty = float(r["top_y"])
                except (KeyError, ValueError):
                    continue
                k = (N, run)
                if k not in g:
                    g[k] = [ty, ty, ty]
                g[k][1] = ty
                g[k][2] = min(g[k][2], ty)
            byN = {}
            for (N, run), (t0, tf, tmin) in g.items():
                byN.setdefault(N, []).append(tf)
            for N in sorted(byN):
                exp = N * 1.0 - 0.5  # topo esperado se intacta (centro do cubo do topo, m)
                finals = byN[N]
                avg_final = sum(finals) / len(finals)
                kept = (avg_final / exp * 100.0) if exp > 0 else 0.0
                verdict = "ESTAVEL" if kept > 90 else ("PARCIAL" if kept > 25 else "COLAPSOU")
                out.append((eng, config, N, exp, avg_final, kept, verdict, len(finals)))
    return out


def fmt_vals(vals, prec=3):
    return ", ".join(f"{v:.{prec}f}" for v in vals)


def md_block_metric(title, unit, groups, prec=3):
    """groups: lista de (label, values). Gera tabela markdown para uma metrica."""
    lines = [f"### {title}", ""]
    lines.append(f"| Config | Runs (valores puros, {unit}) | M | σ | Faixa [M±σ] | Descartados | **Média Final** |")
    lines.append("|---|---|---|---|---|---|---|")
    for label, values in groups:
        s = stats(values)
        if not s:
            lines.append(f"| {label} | (sem dados) | — | — | — | — | — |")
            continue
        dropped = fmt_vals(s["dropped"], prec) if s["dropped"] else "—"
        lines.append(
            f"| {label} | {fmt_vals(values, prec)} | {s['mean']:.{prec}f} | {s['sigma']:.{prec}f} "
            f"| [{s['lo']:.{prec}f}, {s['hi']:.{prec}f}] | {dropped} ({len(s['dropped'])}) "
            f"| **{s['final']:.{prec}f}** |"
        )
    lines.append("")
    return "\n".join(lines)


def build_report():
    torre = collect_torre()
    chuva = collect_chuva()

    md = ["# Resultados — Benchmark de Motores de Física (Grau B)",
          "",
          "> Gerado por `aggregate_stats.py`. Metodologia do enunciado: 10 runs → "
          "M e σ (populacional, ÷n) → descartar fora de [M±σ] → Média Final.",
          "> Torre em **segundos** (tempo até sleep). Chuva em **ms** (physics step) e **FPS**.",
          "> Valores de todas as engines já normalizados às mesmas unidades (m, m/s, ms).",
          ""]

    md.append("## Cenário 1 — A Torre (tempo até repouso total, s)")
    md.append("")
    groups = [(f"{eng} · {cfg} · N={n}", vals) for (eng, cfg, n, vals) in torre]
    if groups:
        md.append(md_block_metric("Tempo até sleep", "s", groups, prec=3))
    else:
        md.append("_(sem CSVs de torre ainda)_\n")

    # Estabilidade da Torre (colapso vs estável) — direto do debug top_y.
    stab = collect_torre_stability()
    if stab:
        md.append("### Estabilidade da Torre (colapso vs estável)")
        md.append("")
        md.append("> `kept%` = altura final do topo ÷ altura esperada se intacta. "
                  "ESTÁVEL >90% · PARCIAL 25–90% · COLAPSOU <25%.")
        md.append("")
        md.append("| Config | N | topo esperado (m) | topo final médio (m) | kept% | Veredito |")
        md.append("|---|---|---|---|---|---|")
        for (eng, cfg, N, exp, avg_final, kept, verdict, nruns) in stab:
            md.append(f"| {eng} · {cfg} | {N} | {exp:.1f} | {avg_final:.2f} | {kept:.0f}% | **{verdict}** |")
        md.append("")

    md.append("## Cenário 2 — A Chuva (Physics Step Time e FPS)")
    md.append("")
    step_groups = [(f"{eng} · {cfg} · {v} esferas", s) for (eng, cfg, v, s, f) in chuva]
    fps_groups = [(f"{eng} · {cfg} · {v} esferas", f) for (eng, cfg, v, s, f) in chuva]
    if step_groups:
        md.append(md_block_metric("Physics Step Time", "ms", step_groups, prec=3))
        md.append(md_block_metric("FPS médio", "fps", fps_groups, prec=1))
    else:
        md.append("_(sem CSVs de chuva ainda)_\n")

    return "\n".join(md)


def main():
    report = build_report()
    if "--print" in sys.argv:
        print(report)
        return
    out_path = os.path.join(RESULTS_DIR, "RESULTS.md")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(report + "\n")
    # Console pode ser cp1252 no Windows; imprime so um resumo ASCII-safe.
    n_lines = report.count("\n") + 1
    print(f"Escrito: {out_path} ({n_lines} linhas). Use --print para ver no console.")


if __name__ == "__main__":
    main()
