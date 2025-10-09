#include "RaidMagtheridonTriggers.h"
#include "Playerbots.h"

bool MagtheridonTrigger::IsActive()
{
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    sPlayerbotAIConfig->sightDistance = 150.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");

    bool result = (magtheridon && magtheridon->IsAlive() ||
                  channeler && channeler->IsAlive());

    if (!result)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;

    return result;
}
