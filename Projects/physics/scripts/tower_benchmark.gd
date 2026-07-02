extends Node3D
#
# Cenario 1 - A Torre
# ---------------------------------
# 1. Instancia N RigidBody3D empilhados (N vem de RunManager.torre_num_cubos_atual()).
# 2. Marca t=0 em _ready (Time.get_ticks_msec).
# 3. A cada _physics_process, conta quantos corpos estao com rb.sleeping == true.
# 4. Quando todos dormem (ou timeout), registra o tempo no RunManager e recarrega a cena.
# 5. O RunManager (AutoLoad) controla quantas runs ja foram executadas.
#
# Justificativa: bounce=0 nos cubos para evitar manter o solver acordado pra sempre.

const ALTURA_CUBO: float = 1.0
# "Caixas perfeitamente empilhadas" conforme spec do assignment:
# cubos com faces em contato exato, sem gap. Eventuais instabilidades
# decorrentes da margem de contato do Jolt (~5mm) ou de impulsos de
# separacao em pilhas altas SAO PARTE DO QUE ESTAMOS MEDINDO ("observar
# se ha jitter excessivo ou se a fisica colapsa").
const CUBE_SPACING: float = 1.0
const OFFSET_INICIAL: float = 0.5

# Diagnostico: registra estado da pilha por physics frame em CSV separado.
const ENABLE_DEBUG: bool = true

var _corpos: Array[RigidBody3D] = []
var _t0_ms: int = 0
var _phys_frame0: int = 0
var _registrado: bool = false

# Metricas diagnosticas acumuladas por run.
var _max_v_observado: float = 0.0
var _t_primeiro_sleep: float = -1.0
var _t_metade_sleep: float = -1.0

# Buffer de samples per-frame (flushed antes do reload).
var _debug_buf: Array[String] = []

# Recursos compartilhados para reduzir custo de criacao.
var _shared_mesh: BoxMesh
var _shared_shape: BoxShape3D
var _shared_mat: PhysicsMaterial

func _ready() -> void:
	var num_cubos: int = RunManager.torre_num_cubos_atual()
	print("[Torre] Run %d/%d iniciando (cubos=%d)" % [
		RunManager.torre_run_atual + 1, RunManager.TORRE_TOTAL_RUNS, num_cubos
	])

	_criar_recursos_compartilhados()
	_criar_ambiente(num_cubos)
	_criar_arena(num_cubos)
	_empilhar_cubos(num_cubos)

	_t0_ms = Time.get_ticks_msec()
	_phys_frame0 = Engine.get_physics_frames()

func _criar_recursos_compartilhados() -> void:
	_shared_mesh = BoxMesh.new()
	_shared_mesh.size = Vector3.ONE
	_shared_shape = BoxShape3D.new()
	_shared_shape.size = Vector3.ONE
	_shared_mat = PhysicsMaterial.new()
	_shared_mat.friction = 0.5
	_shared_mat.bounce = 0.0  # critico para a Torre

func _criar_ambiente(num_cubos: int) -> void:
	var altura: float = num_cubos * CUBE_SPACING

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
	luz.rotation_degrees = Vector3(-50, 35, 0)
	add_child(luz)

	# Camera 3/4 com a torre inteira no frame. Look-point intencionalmente
	# baixo: se a pilha colapsa, ainda vemos a acao perto do chao.
	# dist escala com a altura para que a torre ocupe ~50% do FOV vertical
	# em qualquer numero de cubos (10, 50, 100...).
	# IMPORTANTE: add_child ANTES de look_at - look_at depende de global_transform
	# que so eh valido depois do node estar na arvore.
	# Enquadramento calculado: FOV vertical 60 deg, vista 3/4 (azimute 45 deg).
	# Distancia horizontal por eixo = 0.81*H enquadra a torre inteira ocupando
	# ~80% da altura do frame (H = altura da torre). Mira em 0.42*H (um pouco
	# abaixo do centro, para o colapso perto do chao ficar bem visivel).
	var cam := Camera3D.new()
	cam.fov = 60.0
	cam.keep_aspect = Camera3D.KEEP_HEIGHT
	var dist: float = maxf(15.0, altura * 0.81)
	var cam_y: float = maxf(8.0, altura * 0.50)
	var look_y: float = maxf(4.0, altura * 0.42)
	cam.position = Vector3(dist, cam_y, dist)
	cam.far = 4000.0
	add_child(cam)
	cam.look_at(Vector3(0, look_y, 0), Vector3.UP)

## Arena fechada: chao 60×60m + paredes laterais + teto, todos StaticBody3D.
## Motivo das paredes: quando o solver do Jolt falha em manter a pilha (N=100
## com defaults colapsa em ~todas as runs), cubos podem ser lancados a >50 m/s
## lateralmente. Sem paredes, escapam do chao 60m e caem em queda livre infinita,
## fazendo a run timeout sem informacao util. Com paredes, os cubos ficam
## contidos e eventualmente assentam (mesmo que como pilha de escombros), dando
## medicao real do tempo-ate-sleep mesmo em casos de falha do solver.
## As paredes nao influenciam a fase inicial da simulacao porque estao 30m
## afastadas da base da pilha (cubos so as encontram apos serem ejetados).
func _criar_arena(num_cubos: int) -> void:
	var largura: float = 60.0
	var profundidade: float = 60.0
	var altura: float = maxf(150.0, float(num_cubos) * CUBE_SPACING * 1.5)  # escala com pilha
	var esp: float = 2.0
	var meio_y: float = altura * 0.5

	var mat := PhysicsMaterial.new()
	mat.friction = 0.5
	mat.bounce = 0.0

	# Chao - com mesh visivel
	_criar_parede_arena(Vector3(0, -esp * 0.5, 0), Vector3(largura, esp, profundidade), mat, true)
	# Teto - colisao only
	_criar_parede_arena(Vector3(0, altura + esp * 0.5, 0), Vector3(largura, esp, profundidade), mat, false)
	# Paredes laterais (eixo X) - colisao only
	_criar_parede_arena(Vector3(largura * 0.5 + esp * 0.5, meio_y, 0), Vector3(esp, altura, profundidade), mat, false)
	_criar_parede_arena(Vector3(-largura * 0.5 - esp * 0.5, meio_y, 0), Vector3(esp, altura, profundidade), mat, false)
	# Paredes laterais (eixo Z) - colisao only
	_criar_parede_arena(Vector3(0, meio_y, profundidade * 0.5 + esp * 0.5), Vector3(largura, altura, esp), mat, false)
	_criar_parede_arena(Vector3(0, meio_y, -profundidade * 0.5 - esp * 0.5), Vector3(largura, altura, esp), mat, false)

func _criar_parede_arena(pos: Vector3, tamanho: Vector3, mat: PhysicsMaterial, com_mesh: bool) -> void:
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

	sb.physics_material_override = mat
	add_child(sb)

func _empilhar_cubos(num_cubos: int) -> void:
	for i in range(num_cubos):
		var y: float = OFFSET_INICIAL + i * CUBE_SPACING
		var rb := _criar_cubo(Vector3(0, y, 0))
		_corpos.append(rb)

func _criar_cubo(pos: Vector3) -> RigidBody3D:
	var rb := RigidBody3D.new()
	rb.mass = 1.0
	rb.gravity_scale = 1.0
	rb.can_sleep = true
	rb.physics_material_override = _shared_mat
	rb.position = pos

	var mi := MeshInstance3D.new()
	mi.mesh = _shared_mesh
	rb.add_child(mi)

	var col := CollisionShape3D.new()
	col.shape = _shared_shape
	rb.add_child(col)

	add_child(rb)
	return rb

func _physics_process(_delta: float) -> void:
	if _registrado:
		return

	var elapsed: float = (Time.get_ticks_msec() - _t0_ms) / 1000.0
	var phys_frame: int = Engine.get_physics_frames() - _phys_frame0

	# Snapshot do estado da pilha neste frame.
	var dormindo := 0
	var max_v := 0.0
	var sum_v := 0.0
	var top_y := -INF
	for c in _corpos:
		if c.sleeping:
			dormindo += 1
		var v: float = c.linear_velocity.length()
		if v > max_v:
			max_v = v
		sum_v += v
		if c.position.y > top_y:
			top_y = c.position.y
	var mean_v: float = sum_v / _corpos.size()

	# Metricas globais por run.
	if max_v > _max_v_observado:
		_max_v_observado = max_v
	if dormindo > 0 and _t_primeiro_sleep < 0.0:
		_t_primeiro_sleep = elapsed
	if dormindo >= int(_corpos.size() / 2) and _t_metade_sleep < 0.0:
		_t_metade_sleep = elapsed

	# Debug log per-frame (buffered em memoria, flushed antes do reload).
	if ENABLE_DEBUG:
		var active_objs: int = int(Performance.get_monitor(
			Performance.PHYSICS_3D_ACTIVE_OBJECTS
		))
		_debug_buf.append("%d,%d,%.4f,%d,%d,%d,%.6f,%.6f,%.4f" % [
			RunManager.torre_run_atual + 1,
			RunManager.torre_num_cubos_atual(),
			elapsed, phys_frame,
			dormindo, active_objs, max_v, mean_v, top_y
		])

	var todos_dormindo: bool = dormindo == _corpos.size()
	var timeout: bool = elapsed >= RunManager.TORRE_TIMEOUT_S
	if todos_dormindo or timeout:
		_registrado = true
		if timeout:
			print("[Torre] TIMEOUT em %.2fs (dormindo=%d/%d)" % [
				elapsed, dormindo, _corpos.size()
			])
		if ENABLE_DEBUG:
			RunManager.torre_log_debug(_debug_buf)
		RunManager.torre_registrar(
			elapsed, timeout, _max_v_observado, _t_primeiro_sleep, _t_metade_sleep, phys_frame
		)
		_agendar_proxima()

func _agendar_proxima() -> void:
	# Pequeno buffer para garantir que o CSV foi flushado antes do reload.
	await get_tree().create_timer(0.3).timeout
	RunManager.torre_proxima()
