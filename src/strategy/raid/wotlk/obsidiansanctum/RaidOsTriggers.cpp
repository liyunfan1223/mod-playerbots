#include "RaidOsTriggers.h"

#include "SharedDefines.h"

bool SartharionTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }
    
    return botAI->IsTank(bot);
}

bool FlameTsunamiTrigger::IsActive()
{
    if (botAI->IsTank(bot)) { return false; }

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

    return botAI->IsDps(bot);
}

bool SartharionMeleePositioningTrigger::IsActive()
{
    if (!botAI->IsMelee(bot) || !botAI->IsDps(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return false; }

    Unit* shadron = AI_VALUE2(Unit*, "find target", "shadron");
    Unit* tenebron = AI_VALUE2(Unit*, "find target", "tenebron");
    Unit* vesperon = AI_VALUE2(Unit*, "find target", "vesperon");

    return !(shadron || tenebron || vesperon);
}

bool TwilightPortalEnterTrigger::IsActive()
{
    if (botAI->IsMainTank(bot) || botAI->IsHealAssistantOfIndex(bot, 0)) { return false; }

    // In 25-man, take two healers in. Otherwise just take one
    // if (bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL)
    // {
    //     if (botAI->IsHealAssistantOfIndex(bot, 0) || botAI->IsHealAssistantOfIndex(bot, 1))
    //     {
    //         return false;
    //     }
    // }
    // else
    // {
    //     if (botAI->IsHealAssistantOfIndex(bot, 0))
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