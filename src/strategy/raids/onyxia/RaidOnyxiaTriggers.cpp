#include "RaidOnyxiaTriggers.h"

#include "GenericTriggers.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "strategy/values/NearestNpcsValue.h"

OnyxiaDeepBreathTrigger::OnyxiaDeepBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony deep breath warning") {}

bool OnyxiaDeepBreathTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    return boss->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
           boss->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == 17086;
}

OnyxiaNearTailTrigger::OnyxiaNearTailTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony near tail") {}

bool OnyxiaNearTailTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    float angle = bot->GetAngle(boss);
    float distance = bot->GetDistance(boss);

    return distance < 10.0f && (angle < M_PI / 4 || angle > (2 * M_PI - M_PI / 4));
}

RaidOnyxiaFireballSplashTrigger::RaidOnyxiaFireballSplashTrigger(PlayerbotAI* botAI)
    : Trigger(botAI, "ony fireball splash incoming")
{
}

bool RaidOnyxiaFireballSplashTrigger::IsActive()
{
    Unit* onyxia = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!onyxia || !onyxia->HasUnitState(UNIT_STATE_CASTING))
        return false;

    // Check if Onyxia is casting Fireball
    Spell* currentSpell = onyxia->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || currentSpell->m_spellInfo->Id != 18392)  // 18392 is the classic Fireball ID
        return false;

    GuidVector nearbyUnits = AI_VALUE(GuidVector, "nearest friendly players");

    for (ObjectGuid guid : nearbyUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit == bot || !unit->IsAlive())
            continue;

        if (bot->GetDistance(unit) < 8.0f)
            return true;
    }

    return false;
}
