/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OSTriggers.h"
#include "SharedDefines.h"

bool SartharionTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    return PlayerbotAI::IsTank(bot);
}

bool FlameTsunamiTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit)
        {
            if (unit->GetEntry() == NPC_FLAME_TSUNAMI)
            {
                return true;
            }
        }
    }

    return false;
}

bool TwilightFissureTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit)
        {
            if (unit->GetEntry() == NPC_TWILIGHT_FISSURE)
            {
                return true;
            }
        }
    }

    return false;
}

bool SartharionDpsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    return PlayerbotAI::IsDps(bot);
}

bool SartharionMeleePositioningTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    Unit* shadron = AI_VALUE2(Unit*, "find target", "shadron");
    Unit* tenebron = AI_VALUE2(Unit*, "find target", "tenebron");
    Unit* vesperon = AI_VALUE2(Unit*, "find target", "vesperon");

    return !(shadron || tenebron || vesperon);
}

bool TwilightPortalEnterTrigger::IsActive()
{
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistHealOfIndex(bot, 0)) { return false; }

    // In 25-man, take two healers in. Otherwise just take one
    // if (bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL)
    // {
    //     if (PlayerbotAI::IsAssistHealOfIndex(bot, 0) || PlayerbotAI::IsAssistHealOfIndex(bot, 1))
    //     {
    //         return false;
    //     }
    // }
    // else
    // {
    //     if (PlayerbotAI::IsAssistHealOfIndex(bot, 0))
    //     {
    //         return false;
    //     }
    // }

    // Don't enter portal until drakes are dead
    if (bot->HasAura(SPELL_POWER_OF_SHADRON) ||
        bot->HasAura(SPELL_POWER_OF_TENEBRON) ||
        bot->HasAura(SPELL_POWER_OF_VESPERON))
    {
        return false;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    // GuidVector objects = AI_VALUE(GuidVector, "nearest game objects no los");
    // for (auto& object : objects)
    // {
    //     GameObject* go = botAI->GetGameObject(object);
    //     if (go && go->GetEntry() == GO_TWILIGHT_PORTAL)
    //     {
    //         return true;
    //     }
    // }
    return bool(bot->FindNearestGameObject(GO_TWILIGHT_PORTAL, 100.0f));
}

bool TwilightPortalExitTrigger::IsActive()
{
    return bot->HasAura(SPELL_TWILIGHT_SHIFT) && !AI_VALUE2(Unit*, "find target", "acolyte of shadron");
}
