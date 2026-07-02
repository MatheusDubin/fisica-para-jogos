#include "physics_unreal.h"
#include "Modules/ModuleManager.h"

// Registra este módulo como o módulo primário do jogo. O nome ("physics_unreal")
// deve bater com o nome do módulo no .uproject e com o /Script/physics_unreal
// referenciado no DefaultEngine.ini.
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, physics_unreal, "physics_unreal");
