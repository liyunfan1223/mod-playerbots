/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldResurrectAction.h"

#include <algorithm>

#include "Playerbots.h"
#include "WorldResurrectTargetValue.h"

WorldResurrectAction::WorldResurrectAction(PlayerbotAI* botAI)
    : CastSpellAction(botAI, WorldResurrect::GetSpellName(botAI->GetBot()))
{
}

Unit* WorldResurrectAction::GetTarget() { return WorldResurrect::GetTarget(botAI); }

bool WorldResurrectAction::isUseful() { return WorldResurrect::CanHelp(botAI) && CastSpellAction::isUseful(); }

bool WorldResurrectAction::isPossible()
{
    if (!WorldResurrect::CanHelp(botAI) || !GetTarget())
        return false;

    // The engine checks possibility before prerequisites. Let the normal reach/form actions finish first.
    if (bot->isMoving() || (bot->getClass() == CLASS_DRUID && bot->GetShapeshiftForm() != FORM_NONE))
        return true;

    return CastSpellAction::isPossible();
}

bool WorldResurrectAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target || !WorldResurrect::TryClaim(botAI, target->GetGUID()))
        return false;

    if (bot->isMoving())
        bot->StopMoving();

    if (CastSpellAction::Execute(event))
        return true;

    WorldResurrect::ReleaseClaim(bot->GetGUID());
    return false;
}

std::vector<NextAction> WorldResurrectAction::getPrerequisites()
{
    std::vector<NextAction> prerequisites;
    if (bot->getClass() == CLASS_DRUID)
        prerequisites.emplace_back("caster form");
    else if (bot->getClass() == CLASS_PRIEST)
        prerequisites.emplace_back("remove shadowform");

    prerequisites.emplace_back("reach world resurrect target");
    return prerequisites;
}

ReachWorldResurrectTargetAction::ReachWorldResurrectTargetAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach world resurrect target", botAI->GetRange("spell"))
{
}

Unit* ReachWorldResurrectTargetAction::GetTarget() { return WorldResurrect::GetTarget(botAI); }

bool ReachWorldResurrectTargetAction::isUseful()
{
    return WorldResurrect::CanHelp(botAI) && UpdateRange() && ReachTargetAction::isUseful();
}

bool ReachWorldResurrectTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target || !UpdateRange() || !WorldResurrect::TryClaim(botAI, target->GetGUID()))
        return false;

    if (ReachCombatTo(target, distance))
        return true;

    WorldResurrect::ReleaseClaim(bot->GetGUID());
    return false;
}

bool ReachWorldResurrectTargetAction::UpdateRange()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", WorldResurrect::GetSpellName(bot));
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    distance = std::min(botAI->GetRange("spell"), spellInfo->GetMaxRange(true, bot));
    return distance > 0.0f;
}
