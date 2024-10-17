#include "Playerbots.h"
#include "HallsOfStoneTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

bool KrystallusGroundSlamTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "krystallus");
    if (!boss) { return false; }
    
    // Check both of these... the spell is applied first, debuff later.
    // Neither is active for the full duration so we need to trigger off both
    return bot->HasAura(SPELL_GROUND_SLAM) || bot->HasAura(DEBUFF_GROUND_SLAM);
}

bool SjonnirLightningRingTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sjonnir the ironshaper");
    if (!boss) { return false; }
    
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_RING);
}
