/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterActions.h"
#include "Event.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"

bool CastHuntersMarkAction::isUseful()
{
    return CastDebuffSpellAction::isUseful();
}

bool CastViperStingAction::isUseful()
{
    return AI_VALUE2(uint8, "mana", "self target") < 50 && AI_VALUE2(uint8, "mana", "current target") >= 30;
}

bool CastAspectOfTheCheetahAction::isUseful()
{
    return !botAI->HasAnyAuraOf(GetTarget(), "aspect of the cheetah", "aspect of the pack", nullptr);
}

Value<Unit*>* CastFreezingTrap::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "freezing trap");
}

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
    
    if (bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) && bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets.GetUnitTargetGUID() == AI_VALUE(Unit*, "current target")->GetGUID()) {
        return false;
    }
    return AI_VALUE(uint32, "active spell") != AI_VALUE2(uint32, "spell id", getName());
}

Value<Unit*>* CastScareBeastCcAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", "scare beast");
}

bool CastScareBeastCcAction::Execute(Event event)
{
    return botAI->CastSpell("scare beast", GetTarget());
}

bool CastWingClipAction::isUseful()
{
    return CastSpellAction::isUseful() && !botAI->HasAura(spell, GetTarget());
}

NextAction** CastWingClipAction::getPrerequisites()
{
    return nullptr;
}

// bool CastRaptorStrikeAction::isUseful()
// {
//     return CastMeleeSpellAction::isUseful() && botAI->HasStrategy("close", BOT_STATE_COMBAT);
// }
