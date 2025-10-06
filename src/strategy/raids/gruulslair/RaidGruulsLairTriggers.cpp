#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

bool HighKingMaulgarTrigger::IsActive()
{
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    sPlayerbotAIConfig->sightDistance = 150.0f;

    bool foundBosses = IsAnyOgreBossAlive(botAI);

    if (!foundBosses)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;

    return foundBosses;
}

bool GruulTheDragonkillerTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    
    return gruul && gruul->IsAlive();
}
