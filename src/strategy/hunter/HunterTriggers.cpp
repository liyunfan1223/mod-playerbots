/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterTriggers.h"

#include "GenericTriggers.h"
#include "HunterActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"

bool HunterAspectOfTheHawkTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() && !botAI->HasAura("aspect of the hawk", target) &&
           !botAI->HasAura("aspect of the dragonhawk", target) &&
           (!AI_VALUE2(bool, "has mana", "self target") || AI_VALUE2(uint8, "mana", "self target") > 70);
}

bool HunterNoStingsActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return DebuffTrigger::IsActive() && target && !botAI->HasAura("serpent sting", target, false, true) &&
           !botAI->HasAura("scorpid sting", target, false, true) && !botAI->HasAura("viper sting", target, false, true);
}

bool HuntersPetDeadTrigger::IsActive()
{
    // Unit* pet = AI_VALUE(Unit*, "pet target");
    // return pet && AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target");
    return AI_VALUE(bool, "pet dead") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HuntersPetLowHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < 40 && !AI_VALUE2(bool, "dead", "pet target") &&
           !AI_VALUE2(bool, "mounted", "self target");
}

bool HuntersPetMediumHealthTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    return pet && AI_VALUE2(uint8, "health", "pet target") < sPlayerbotAIConfig->mediumHealth &&
           !AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HunterPetNotHappy::IsActive()
{
    return !AI_VALUE(bool, "pet happy") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HunterAspectOfTheViperTrigger::IsActive()
{
    return SpellTrigger::IsActive() && !botAI->HasAura(spell, GetTarget()) &&
           AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->lowMana;
    ;
}

bool HunterAspectOfThePackTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("aspect of the cheetah", GetTarget());
};

bool HunterLowAmmoTrigger::IsActive()
{
    return bot->GetGroup() && (AI_VALUE2(uint32, "item count", "ammo") < 100) &&
           (AI_VALUE2(uint32, "item count", "ammo") > 0);
}

bool HunterHasAmmoTrigger::IsActive() { return !AmmoCountTrigger::IsActive(); }

bool SwitchToRangedTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return botAI->HasStrategy("close", BOT_STATE_COMBAT) && target &&
           (target->GetVictim() != bot &&
            sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float, "distance", "current target"), 8.0f));
}

bool SwitchToMeleeTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return botAI->HasStrategy("ranged", BOT_STATE_COMBAT) && target &&
           (target->GetVictim() == bot &&
            sServerFacade->IsDistanceLessOrEqualThan(AI_VALUE2(float, "distance", "current target"), 8.0f));
}
