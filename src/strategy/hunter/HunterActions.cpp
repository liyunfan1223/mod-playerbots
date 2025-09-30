/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HunterActions.h"

#include "Event.h"
#include "GenericSpellActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool CastViperStingAction::isUseful()
{
    return CastAuraSpellAction::isUseful() && AI_VALUE2(uint8, "mana", "self target") < 50 &&
           AI_VALUE2(uint8, "mana", "current target") >= 30;
}

bool CastAspectOfTheCheetahAction::isUseful()
{
    return !botAI->HasAnyAuraOf(GetTarget(), "aspect of the cheetah", "aspect of the pack", nullptr);
}

bool CastAspectOfTheHawkAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    if (bot->HasSpell(61846) || bot->HasSpell(61847))  // Aspect of the Dragonhawk spell IDs
        return false;
    return true;
}

bool CastArcaneShotAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    if (bot->HasSpell(53301) || bot->HasSpell(60051) || bot->HasSpell(60052) || bot->HasSpell(60053))  // Explosive Shot spell IDs
        return false;

    // Armor Penetration rating check - will not cast Arcane Shot above 435 ArP
    int32 armorPenRating = bot->GetUInt32Value(PLAYER_FIELD_COMBAT_RATING_1) + bot->GetUInt32Value(CR_ARMOR_PENETRATION);
    if (armorPenRating > 435)
        return false;

    return true;
}

bool CastImmolationTrapAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    if (bot->HasSpell(13813) || bot->HasSpell(14316) || bot->HasSpell(14317) || bot->HasSpell(27025) || bot->HasSpell(49066) || bot->HasSpell(49067)) // Explosive Trap spell IDs
        return false;
    return true;
}

Value<Unit*>* CastFreezingTrap::GetTargetValue() { return context->GetValue<Unit*>("cc target", "freezing trap"); }

bool FeedPetAction::Execute(Event event)
{
    if (Pet* pet = bot->GetPet())
        if (pet->getPetType() == HUNTER_PET && pet->GetHappinessState() != HAPPY)
            pet->SetPower(POWER_HAPPINESS, pet->GetMaxPower(Powers(POWER_HAPPINESS)));

    return true;
}

bool CastAutoShotAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (AI_VALUE(Unit*, "current target") && bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) &&
        bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets.GetUnitTargetGUID() ==
            AI_VALUE(Unit*, "current target")->GetGUID())
    {
        return false;
    }
    return AI_VALUE(uint32, "active spell") != AI_VALUE2(uint32, "spell id", getName());
}

bool CastDisengageAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    // can cast spell check passed in isUseful()
    bot->SetOrientation(bot->GetAngle(target));
    return CastSpellAction::Execute(event);
}

bool CastDisengageAction::isUseful()
{
    return !botAI->HasStrategy("trap weave", BOT_STATE_COMBAT);
}

Value<Unit*>* CastScareBeastCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "scare beast"); }

bool CastScareBeastCcAction::Execute(Event event) { return botAI->CastSpell("scare beast", GetTarget()); }

bool CastWingClipAction::isUseful() { return CastSpellAction::isUseful() && !botAI->HasAura(spell, GetTarget()); }

NextAction** CastWingClipAction::getPrerequisites() { return nullptr; }

// bool CastRaptorStrikeAction::isUseful()
// {
//     return CastMeleeSpellAction::isUseful() && botAI->HasStrategy("close", BOT_STATE_COMBAT);
// }
