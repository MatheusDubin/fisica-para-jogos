extends Node

# ============================================================
#  CONFIGURACAO DE BENCHMARKS
# ============================================================

# Cenario 1 - Torre
# Canonico: [100] (10 runs em N=100, com arena fechada).
# Para stress-test, usar varios N (e.g. [5,10,15,20,25,30,35,40,45,50]).
const TORRE_N_VALUES: Array[int] = [100]
const TORRE_TOTAL_RUNS: int = 10
const TORRE_TIMEOUT_S: float = 60.0
const TORRE_CSV: String = "user://torre_godot.csv"
const TORRE_DEBUG_CSV: String = "user://torre_godot_debug.csv"

# Cenario 2 - Chuva
const CHUVA_TOTAL_RUNS: int = 10
const CHUVA_JANELA_S: float = 10.0
const CHUVA_AQUECIMENTO_S: float = 0.5
# Sessao atual: rodando APENAS 10000 com buffer Jolt aumentado.
# Para a sessao completa, restaurar para [1000, 5000, 10000].
const CHUVA_VARIACOES: Array[int] = [10000]
const CHUVA_CSV: String = "user://chuva_godot.csv"

# ============================================================
#  ESTADO PERSISTENTE ENTRE RELOADS DE CENA
# ============================================================

var torre_run_atual: int = 0
var torre_n_idx: int = 0
var torre_resultados: Array = []  # cada item: {n: int, tempo: float, timeout: bool}

var chuva_variacao_idx: int = 0
var chuva_run_atual: int = 0
var chuva_resultados: Array = []

# ============================================================

func _ready() -> void:
	print("[RunManager] Godot %s" % Engine.get_version_info()["string"])
	print("[RunManager] Physics engine: %s" % ProjectSettings.get_setting("physics/3d/physics_engine"))
	print("[RunManager] Ticks/s: %s" % ProjectSettings.get_setting("physics/common/physics_ticks_per_second"))
	print("[RunManager] User data dir: %s" % OS.get_user_data_dir())

# ------------- TORRE -------------

func torre_num_cubos_atual() -> int:
	if torre_n_idx >= TORRE_N_VALUES.size():
		return 0
	return TORRE_N_VALUES[torre_n_idx]

func torre_registrar(
	tempo_ate_sleep: float,
	timeout_atingido: bool,
	max_v_observado: float,
	t_primeiro_sleep: float,
	t_metade_sleep: float,
	phys_frames: int
) -> void:
	# Truncate CSV apenas na PRIMEIRA run da PRIMEIRA variacao de N da sessao.
	var primeira := torre_run_atual == 0 and torre_n_idx == 0
	torre_run_atual += 1
	var n := torre_num_cubos_atual()
	torre_resultados.append({"n": n, "tempo": tempo_ate_sleep, "timeout": timeout_atingido})
	var flag := 1 if timeout_atingido else 0
	_csv_escrever(
		TORRE_CSV,
		"run,num_cubos,tempo_ate_sleep_s,timeout,phys_frames,max_v,t_primeiro_sleep,t_metade_sleep",
		"%d,%d,%.4f,%d,%d,%.6f,%.4f,%.4f" % [
			torre_run_atual, n, tempo_ate_sleep, flag,
			phys_frames, max_v_observado, t_primeiro_sleep, t_metade_sleep
		],
		primeira
	)
	print("[Torre N=%d] Run %d/%d -> %.4fs (frames=%d, max_v=%.4f, 1st=%.2f, half=%.2f)%s" % [
		n, torre_run_atual, TORRE_TOTAL_RUNS, tempo_ate_sleep,
		phys_frames, max_v_observado, t_primeiro_sleep, t_metade_sleep,
		" (TIMEOUT)" if timeout_atingido else ""
	])

func torre_log_debug(linhas: Array[String]) -> void:
	if linhas.is_empty():
		return
	# Truncate apenas na primeira run da primeira variacao de N da sessao.
	var primeira := torre_run_atual == 0 and torre_n_idx == 0
	var arquivo: FileAccess
	if primeira:
		arquivo = FileAccess.open(TORRE_DEBUG_CSV, FileAccess.WRITE)
		if arquivo == null:
			push_error("Falha ao criar debug CSV")
			return
		arquivo.store_line("run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y")
	else:
		arquivo = FileAccess.open(TORRE_DEBUG_CSV, FileAccess.READ_WRITE)
		if arquivo == null:
			arquivo = FileAccess.open(TORRE_DEBUG_CSV, FileAccess.WRITE)
			if arquivo == null:
				push_error("Falha ao abrir debug CSV")
				return
			arquivo.store_line("run,num_cubos,t_s,phys_frame,asleep,active_objs,max_v,mean_v,top_y")
		arquivo.seek_end()
	for l in linhas:
		arquivo.store_line(l)
	arquivo.close()

func torre_proxima() -> void:
	if torre_run_atual >= TORRE_TOTAL_RUNS:
		torre_run_atual = 0
		torre_n_idx += 1
		if torre_n_idx >= TORRE_N_VALUES.size():
			_resumo_torre()
			get_tree().quit()
			return
		print("[Torre] Avancando para N=%d" % torre_num_cubos_atual())
	get_tree().reload_current_scene()

func _resumo_torre() -> void:
	if torre_resultados.is_empty():
		return
	# Agrupa por N para um resumo legivel no Output.
	var por_n: Dictionary = {}
	for r in torre_resultados:
		var n: int = r["n"]
		if not por_n.has(n):
			por_n[n] = {"tempos": [] as Array[float], "timeouts": 0}
		por_n[n]["tempos"].append(r["tempo"])
		if r["timeout"]:
			por_n[n]["timeouts"] += 1

	print("[Torre] CONCLUIDO. Stress-test summary:")
	var ns: Array = por_n.keys()
	ns.sort()
	for n in ns:
		var tempos: Array = por_n[n]["tempos"]
		var soma := 0.0
		for t in tempos:
			soma += t
		var media := soma / tempos.size()
		var soma_quad := 0.0
		for t in tempos:
			soma_quad += (t - media) * (t - media)
		var desvio := sqrt(soma_quad / tempos.size())
		print("  N=%d: runs=%d media=%.3fs sigma=%.3f timeouts=%d" % [
			n, tempos.size(), media, desvio, por_n[n]["timeouts"]
		])

# ------------- CHUVA -------------

func chuva_variacao_atual() -> int:
	if chuva_variacao_idx >= CHUVA_VARIACOES.size():
		return -1
	return CHUVA_VARIACOES[chuva_variacao_idx]

func chuva_registrar(variacao: int, step_ms_medio: float, step_ms_max: float, fps_medio: float, amostras: int) -> void:
	var primeira := chuva_run_atual == 0 and chuva_variacao_idx == 0
	chuva_run_atual += 1
	chuva_resultados.append({
		"variacao": variacao,
		"run": chuva_run_atual,
		"step_ms": step_ms_medio,
		"fps": fps_medio,
	})
	_csv_escrever(
		CHUVA_CSV,
		"run,variacao,physics_step_ms_medio,physics_step_ms_max,fps_medio,amostras",
		"%d,%d,%.4f,%.4f,%.2f,%d" % [
			chuva_run_atual, variacao, step_ms_medio, step_ms_max, fps_medio, amostras
		],
		primeira
	)
	print("[Chuva %d] Run %d/%d step=%.4fms (max %.4f) fps=%.2f n=%d" % [
		variacao, chuva_run_atual, CHUVA_TOTAL_RUNS, step_ms_medio, step_ms_max, fps_medio, amostras
	])

func chuva_proxima() -> void:
	# Mantido para compat; nao usado pelo novo rain_benchmark (que evita
	# reload_current_scene para nao quebrar o lifecycle de bodies do Jolt em N=10k).
	if chuva_run_atual >= CHUVA_TOTAL_RUNS:
		chuva_run_atual = 0
		chuva_variacao_idx += 1
		if chuva_variacao_idx >= CHUVA_VARIACOES.size():
			print("[Chuva] CONCLUIDO. %d entradas no CSV." % chuva_resultados.size())
			get_tree().quit()
			return
		print("[Chuva] Avancando para variacao=%d" % chuva_variacao_atual())
	get_tree().reload_current_scene()

# Avanca estado sem recarregar a cena. Retorna true se ha mais runs a executar.
# Quando todas as variacoes acabam, faz quit() e retorna false.
func chuva_avancar_sem_reload() -> bool:
	if chuva_run_atual >= CHUVA_TOTAL_RUNS:
		chuva_run_atual = 0
		chuva_variacao_idx += 1
		if chuva_variacao_idx >= CHUVA_VARIACOES.size():
			print("[Chuva] CONCLUIDO. %d entradas no CSV." % chuva_resultados.size())
			get_tree().quit()
			return false
		print("[Chuva] Avancando para variacao=%d" % chuva_variacao_atual())
	return true

# ------------- CSV -------------
# `truncate=true` reescreve o arquivo do zero (com header). Usado para a primeira run
# da sessao, para nao acumular dados de sessoes antigas.

func _csv_escrever(caminho: String, header: String, linha: String, truncate: bool) -> void:
	var arquivo: FileAccess
	if truncate:
		arquivo = FileAccess.open(caminho, FileAccess.WRITE)
		if arquivo == null:
			push_error("Falha ao criar CSV em %s (erro %d)" % [caminho, FileAccess.get_open_error()])
			return
		arquivo.store_line(header)
	else:
		arquivo = FileAccess.open(caminho, FileAccess.READ_WRITE)
		if arquivo == null:
			arquivo = FileAccess.open(caminho, FileAccess.WRITE)
			if arquivo == null:
				push_error("Falha ao criar CSV em %s (erro %d)" % [caminho, FileAccess.get_open_error()])
				return
			arquivo.store_line(header)
		arquivo.seek_end()
	arquivo.store_line(linha)
	arquivo.close()
