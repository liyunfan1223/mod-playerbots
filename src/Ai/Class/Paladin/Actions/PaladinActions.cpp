/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PaladinActions.h"
#include "AiFactory.h"
#include "Event.h"
#include "PaladinHelper.h"
#include "Pet.h"
#include "Playerbots.h"
#include "SharedDefines.h"

bool CastSealSpellAction::isUseful()
{
    return AI_VALUE2(bool, "combat", "self target");
}

Value<Unit*>* CastTurnUndeadAction::GetTargetValue()
{
    return context->GetValue<Unit*>("cc target", getName());
}

Unit* CastHandOfFreedomOnPartyAction::GetTarget()
{
    bool const selfImpaired = botAI->IsMovementImpaired(bot);
    bool const hasSelfHand =
        selfImpaired && ai::paladin::HasAnyPaladinHandFromCaster(bot, bot);

    if (!bot->GetGroup())
    {
        if (selfImpaired && !hasSelfHand)
            return bot;

        return nullptr;
    }

    if (selfImpaired && !hasSelfHand)
        return bot;

    return CastBuffSpellAction::GetTarget();
}

Value<Unit*>* CastHandOfFreedomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member snared target");
}

bool CastHandOfFreedomOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return CastBuffSpellAction::isUseful() &&
           !ai::paladin::HasAnyPaladinHandFromCaster(target, bot);
}

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
        return nullptr;

    return current_target->GetVictim();
}

bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}
