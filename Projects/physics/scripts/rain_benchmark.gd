extends Node3D
#
# Cenario 2 - Chuva de Corpos (in-place, sem scene reload)
# ---------------------------------
# Ambiente (cam, luz, sky) e caixa estatica criados UMA VEZ em _ready.
# Para cada run:
#   1. Spawna _num_objetos esferas, registrando-as em _spheres.
#   2. Aquecimento de CHUVA_AQUECIMENTO_S (descartado).
#   3. Coleta de Performance.TIME_PHYSICS_PROCESS e FPS por CHUVA_JANELA_S.
#   4. Registra media/max no RunManager + CSV.
#   5. queue_free() em todas as esferas, aguarda alguns physics frames para
#      garantir cleanup do Jolt, e inicia a proxima run dentro da mesma cena.
#
# Motivo para NAO usar get_tree().reload_current_scene() aqui: em N=10k,
# o ciclo de destruicao/criacao de 10000 RigidBody3D no scene reload
# acumula estado interno do Jolt entre runs e causa crash na 2a run.
# Mantendo a mesma scene tree e fazendo cleanup explicito, evitamos isso.

const SPHERE_RADIUS: float = 0.5
const SPHERE_DIAM: float = 1.0
const SPACING: float = 1.15  # > diametro para evitar overlap inicial

# Janela por TEMPO DE SIMULACAO (contagem de physics frames), NAO wall-clock:
# garante que todas as engines medem o MESMO estado fisico (queda + pilha) na
# mesma duracao simulada, independente de rodar acima/abaixo do tempo real.
const WARMUP_STEPS: int = 25    # ~0.5s simulados (descartado; curto de proposito)
const WINDOW_STEPS: int = 500   # 10s simulados (coleta) - a correcao de verdade

var _num_objetos: int = 0

var _spheres: Array[RigidBody3D] = []

var _step_ms: Array[float] = []
var _fps: Array[float] = []

var _t0_ms: int = 0
var _t_coleta_ms: int = 0
var _steps: int = 0
var _aquecendo: bool = true
var _coletando: bool = false
var _registrado: bool = false

# Recursos compartilhados (1 unico SphereMesh/Shape/Material para todas).
var _shared_mesh: SphereMesh
var _shared_shape: SphereShape3D
var _shared_mat: PhysicsMaterial

func _ready() -> void:
	_criar_recursos_compartilhados()
	_criar_ambiente()
	_criar_caixa()
	# Inicia o ciclo de runs. A primeira run sera a CHUVA_VARIACOES[0]
	# (no estado atual da sessao - chuva_variacao_idx ja pode estar avancado).
	_iniciar_proxima_run()

func _iniciar_proxima_run() -> void:
	_num_objetos = RunManager.chuva_variacao_atual()
	if _num_objetos <= 0:
		print("[Chuva] Sem variacao ativa - encerrando.")
		get_tree().quit()
		return

	print("[Chuva] Variacao=%d, Run %d/%d (aquecimento=%.2fs, janela=%.2fs)" % [
		_num_objetos,
		RunManager.chuva_run_atual + 1,
		RunManager.CHUVA_TOTAL_RUNS,
		RunManager.CHUVA_AQUECIMENTO_S,
		RunManager.CHUVA_JANELA_S,
	])

	_step_ms.clear()
	_fps.clear()
	_steps = 0
	_aquecendo = true
	_coletando = false
	_registrado = false

	var t_spawn: int = Time.get_ticks_msec()
	_spawn_esferas()
	print("[Chuva] _spawn de %d esferas em %d ms" % [
		_num_objetos, Time.get_ticks_msec() - t_spawn
	])

	_t0_ms = Time.get_ticks_msec()

func _criar_recursos_compartilhados() -> void:
	_shared_mesh = SphereMesh.new()
	_shared_mesh.radius = SPHERE_RADIUS
	_shared_mesh.height = SPHERE_DIAM
	_shared_mesh.radial_segments = 8
	_shared_mesh.rings = 4

	_shared_shape = SphereShape3D.new()
	_shared_shape.radius = SPHERE_RADIUS

	_shared_mat = PhysicsMaterial.new()
	_shared_mat.friction = 0.4
	_shared_mat.bounce = 0.3

func _criar_ambiente() -> void:
	var we := WorldEnvironment.new()
	var env := Environment.new()
	env.background_mode = Environment.BG_SKY
	var sky := Sky.new()
	sky.sky_material = ProceduralSkyMaterial.new()
	env.sky = sky
	env.ambient_light_source = Environment.AMBIENT_SOURCE_SKY
	we.environment = env
	add_child(we)

	var luz := DirectionalLight3D.new()
	luz.rotation_degrees = Vector3(-50, 30, 0)
	add_child(luz)

	# Camera: add_child ANTES de look_at para que look_at use o global_transform
	# corretamente.
	var cam := Camera3D.new()
	cam.position = Vector3(55, 35, 55)
	cam.far = 1000.0
	add_child(cam)
	cam.look_at(Vector3(0, 20, 0), Vector3.UP)

# Caixa fechada (6 paredes estaticas). Criada UMA VEZ; persiste entre runs.
func _criar_caixa() -> void:
	var largura := 35.0
	var altura := 80.0
	var profundidade := 35.0
	var esp := 2.0
	var meio_y := altura * 0.5

	_criar_parede(Vector3(0, -esp * 0.5, 0), Vector3(largura, esp, profundidade), true)
	_criar_parede(Vector3(0, altura + esp * 0.5, 0), Vector3(largura, esp, profundidade), false)
	_criar_parede(Vector3(largura * 0.5 + esp * 0.5, meio_y, 0), Vector3(esp, altura, profundidade), false)
	_criar_parede(Vector3(-largura * 0.5 - esp * 0.5, meio_y, 0), Vector3(esp, altura, profundidade), false)
	_criar_parede(Vector3(0, meio_y, profundidade * 0.5 + esp * 0.5), Vector3(largura, altura, esp), false)
	_criar_parede(Vector3(0, meio_y, -profundidade * 0.5 - esp * 0.5), Vector3(largura, altura, esp), false)

func _criar_parede(pos: Vector3, tamanho: Vector3, com_mesh: bool) -> void:
	var sb := StaticBody3D.new()
	sb.position = pos

	var col := CollisionShape3D.new()
	var shape := BoxShape3D.new()
	shape.size = tamanho
	col.shape = shape
	sb.add_child(col)

	if com_mesh:
		var mi := MeshInstance3D.new()
		var mesh := BoxMesh.new()
		mesh.size = tamanho
		mi.mesh = mesh
		sb.add_child(mi)

	var mat := PhysicsMaterial.new()
	mat.friction = 0.4
	mat.bounce = 0.3
	sb.physics_material_override = mat

	add_child(sb)

func _spawn_esferas() -> void:
	var por_lado: int = max(1, int(ceil(pow(float(_num_objetos), 1.0 / 3.0))))
	var origem_x: float = -(por_lado - 1) * SPACING * 0.5
	var origem_z: float = -(por_lado - 1) * SPACING * 0.5
	var origem_y: float = 25.0

	var rng := RandomNumberGenerator.new()
	rng.seed = 0xBEEF + RunManager.chuva_run_atual

	var n := 0
	for ix in range(por_lado):
		for iy in range(por_lado):
			for iz in range(por_lado):
				if n >= _num_objetos:
					return
				var jitter := Vector3(
					rng.randf_range(-0.03, 0.03),
					rng.randf_range(-0.03, 0.03),
					rng.randf_range(-0.03, 0.03),
				)
				var pos := Vector3(
					origem_x + ix * SPACING,
					origem_y + iy * SPACING,
					origem_z + iz * SPACING,
				) + jitter
				_criar_esfera(pos)
				n += 1

func _criar_esfera(pos: Vector3) -> void:
	var rb := RigidBody3D.new()
	rb.mass = 1.0
	rb.physics_material_override = _shared_mat
	rb.position = pos

	var mi := MeshInstance3D.new()
	mi.mesh = _shared_mesh
	rb.add_child(mi)

	var col := CollisionShape3D.new()
	col.shape = _shared_shape
	rb.add_child(col)

	add_child(rb)
	_spheres.append(rb)

func _physics_process(_delta: float) -> void:
	if _registrado:
		return

	_steps += 1

	if _aquecendo:
		if _steps >= WARMUP_STEPS:
			_aquecendo = false
			_coletando = true
		return

	if _coletando:
		var step_ms: float = Performance.get_monitor(Performance.TIME_PHYSICS_PROCESS) * 1000.0
		var fps: float = Engine.get_frames_per_second()
		_step_ms.append(step_ms)
		_fps.append(fps)

		if _step_ms.size() % 100 == 1:
			print("[Chuva %d] coleta sim_t=%.1fs step=%.2fms fps=%.1f n=%d" % [
				_num_objetos, _step_ms.size() * 0.02, step_ms, fps, _step_ms.size()
			])
		if _step_ms.size() >= WINDOW_STEPS:
			_finalizar()

func _finalizar() -> void:
	_registrado = true
	_coletando = false

	var step_medio: float = _media(_step_ms)
	var step_max: float = _maximo(_step_ms)
	var fps_medio: float = _media(_fps)
	var amostras: int = _step_ms.size()

	RunManager.chuva_registrar(_num_objetos, step_medio, step_max, fps_medio, amostras)

	_ciclar_para_proxima_run()

func _ciclar_para_proxima_run() -> void:
	# Libera as esferas da run atual em duas fases para dar tempo ao Jolt
	# de processar o cleanup antes do proximo spawn.
	print("[Chuva] Liberando %d esferas..." % _spheres.size())
	var t_free: int = Time.get_ticks_msec()
	for sphere in _spheres:
		sphere.queue_free()
	_spheres.clear()

	# Espera physics frames para garantir que o queue_free foi processado
	# e o Jolt removeu os bodies do mundo.
	await get_tree().physics_frame
	await get_tree().physics_frame
	await get_tree().create_timer(0.3).timeout
	print("[Chuva] Cleanup levou %d ms" % (Time.get_ticks_msec() - t_free))

	if RunManager.chuva_avancar_sem_reload():
		_iniciar_proxima_run()
	# else: chuva_avancar_sem_reload ja chamou quit() quando acabou tudo.

func _media(arr: Array[float]) -> float:
	if arr.is_empty():
		return 0.0
	var s := 0.0
	for v in arr:
		s += v
	return s / arr.size()

func _maximo(arr: Array[float]) -> float:
	if arr.is_empty():
		return 0.0
	var m: float = arr[0]
	for v in arr:
		if v > m:
			m = v
	return m
