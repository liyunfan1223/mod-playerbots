#include "RaidVoATriggers.h"

#include "EventMap.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool EmalonMarkBossTrigger::IsActive()
{
    // Only tank bot can mark target
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if boss have skull mark
    Group* group = bot->GetGroup();
    int8 skullIndex = 7;  // Skull
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    if (currentSkullTarget == boss->GetGUID())
    {
        return false;
    }

    // Check if there is any overcharged minion
    Unit* overchargedMinion = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_TEMPEST_MINION && unit->HasAura(AURA_OVERCHARGE))
        {
            overchargedMinion = unit;
            break;
        }
    }
    if (overchargedMinion)
    {
        return false;
    }

    return true;
}

bool EmalonLightingNovaTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Tank dont need to move
    if (botAI->IsTank(bot))
    {
        return false;
    }

    // Check if boss is casting Lightning Nova
    bool isCasting = boss->HasUnitState(UNIT_STATE_CASTING);
    bool isLightingNova = boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA_10_MAN) ||
                          boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA_25_MAN);
    return isCasting && isLightingNova;
}

bool EmalonOverchargeTrigger::IsActive()
{
    // Only tank bot can mark target
    if (!botAI->IsTank(bot))
    {
        return false;
    }

    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if there is any overcharged minion
    Unit* overchargedMinion = nullptr;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_TEMPEST_MINION && unit->HasAura(AURA_OVERCHARGE))
        {
            overchargedMinion = unit;
            break;
        }
    }
    if (!overchargedMinion)
    {
        return false;
    }

    // Check if minion have skull mark
    Group* group = bot->GetGroup();
    int8 skullIndex = 7;  // Skull
    ObjectGuid currentSkullTarget = group->GetTargetIcon(skullIndex);
    if (currentSkullTarget == overchargedMinion->GetGUID())
    {
        return false;
    }

    return true;
}

bool EmalonFallFromFloorTrigger::IsActive()
{
    // Check boss and it is alive
    Unit* boss = AI_VALUE2(Unit*, "find target", "emalon the storm watcher");
    if (!boss || !boss->IsAlive())
    {
        return false;
    }

    // Check if bot is on the floor
    return bot->GetPositionZ() < 80.0f;
}
