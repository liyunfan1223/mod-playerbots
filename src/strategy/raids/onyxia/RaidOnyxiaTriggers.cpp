#include "RaidOnyxiaTriggers.h"

#include "GenericTriggers.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "strategy/values/NearestNpcsValue.h"

OnyxiaPhase1StartTrigger::OnyxiaPhase1StartTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony phase1 start") {}

bool OnyxiaPhase1StartTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    return boss && boss->GetHealthPct() > 65;
}

OnyxiaPhase2StartTrigger::OnyxiaPhase2StartTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony phase2 start") {}

bool OnyxiaPhase2StartTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    return boss && boss->GetHealthPct() <= 65 && boss->GetHealthPct() > 40;
}

OnyxiaPhase3StartTrigger::OnyxiaPhase3StartTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony phase3 start") {}

bool OnyxiaPhase3StartTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    return boss && boss->GetHealthPct() <= 40;
}

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

OnyxiaFearedTrigger::OnyxiaFearedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony feared") {}

bool OnyxiaFearedTrigger::IsActive()
{
    return bot->HasAura(18431);  // Bellowing Roar
}

OnyxiaLavaPulseTrigger::OnyxiaLavaPulseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony lava pulse") {}

bool OnyxiaLavaPulseTrigger::IsActive()
{
    return bot->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE) && bot->IsInCombat() && bot->HasAura(22191);
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
