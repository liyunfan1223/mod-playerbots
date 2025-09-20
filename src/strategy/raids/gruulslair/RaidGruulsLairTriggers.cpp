#include "RaidGruulsLairTriggers.h"
#include "Playerbots.h"

bool HighKingMaulgarTrigger::IsActive()
{

    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    
    sPlayerbotAIConfig->sightDistance = 150.0f;
    
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    
    bool foundBosses = (maulgar && maulgar->IsAlive()) ||
                      (kiggler && kiggler->IsAlive()) ||
                      (krosh && krosh->IsAlive()) ||
                      (olm && olm->IsAlive()) ||
                      (blindeye && blindeye->IsAlive());

    if (!foundBosses)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;
    
    return foundBosses;
}

bool GruulTheDragonkillerTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    
    return gruul && gruul->IsAlive();
}
