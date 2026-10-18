/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Kologarn.h"

#include "GameObject.h"
#include "Group.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldData.h"
#include "UldEncounter_Kologarn.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>

bool KologarnBodyTankTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI))
        return false;

    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return false;

    if (!IsKologarnBodyTank(botAI, bot))
        return false;

    return AI_VALUE(Unit*, "current target") != kologarn || !bot->IsWithinMeleeRange(kologarn);
}

bool KologarnOffTankTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI) || !IsKologarnOffTank(botAI, bot))
        return false;

    // Rubble duty owns the off-tank whenever any are up; this is only the idle case.
    if (KologarnHasRubble(botAI))
        return false;

    Unit* target = GetKologarnOffTankTarget(botAI, bot);
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool KologarnRubbleTankTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI) || !IsKologarnOffTank(botAI, bot))
        return false;

    return GetKologarnNearestRubble(botAI, bot) != nullptr;
}

bool KologarnDpsTargetTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI))
        return false;

    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return false;

    Unit* target = GetKologarnDpsTarget(botAI, bot);
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool KologarnSmashSwapTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI))
        return false;

    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return false;

    // Only the main tank and first assist tank trade the body.
    bool const isMainTank = PlayerbotAI::IsMainTank(bot);
    if (!isMainTank && !PlayerbotAI::IsAssistTankOfIndex(bot, 0))
        return false;

    // bot must be the one not currently holding it.
    Unit* activeTank = kologarn->GetVictim();
    if (!activeTank || activeTank == bot)
        return false;

    Player* activeTankPlayer = activeTank->ToPlayer();
    if (!activeTankPlayer)
        return false;

    bool const partnerIsSwapTank = isMainTank ? PlayerbotAI::IsAssistTankOfIndex(activeTankPlayer, 0)
                                              : PlayerbotAI::IsMainTank(activeTankPlayer);
    if (!partnerIsSwapTank)
        return false;

    if (GetKologarnCrunchArmorStacks(activeTank) < ULDUAR_KOLOGARN_CRUNCH_ARMOR_SWAP_STACKS)
        return false;

    // Strictly fewer, not an absolute cap: Crunch Armor lasts 45s against a 14s Smash timer, so
    // stacks never fully clear and a cap would deadlock both tanks at 2 and stop swapping for good.
    return GetKologarnCrunchArmorStacks(bot) < GetKologarnCrunchArmorStacks(activeTank);
}

bool KologarnBodyUncoveredTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI))
        return false;

    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn || !bot->IsAlive())
        return false;

    Unit* victim = kologarn->GetVictim();
    if (victim && victim->IsAlive() && victim->IsWithinMeleeRange(kologarn))
        return false;

    if (!PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsMelee(bot))
        return false;

    // Breath needs the victim out of melee *and* SelectNearbyTarget finding nobody else close, so any
    // body in melee stops it. This bot standing there is already the fix.
    if (bot->IsWithinMeleeRange(kologarn))
        return false;

    // Nearest tank covers it, and only if no tank is left does the nearest melee step in. Without
    // this every melee in the raid would pile onto the body at once.
    bool const botIsTank = PlayerbotAI::IsTank(bot);
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float const myDistance = bot->GetExactDist2d(kologarn);
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        bool const memberIsTank = PlayerbotAI::IsTank(member);
        if (!memberIsTank && !PlayerbotAI::IsMelee(member))
            continue;

        // A living tank always outranks melee, whatever the distance.
        if (memberIsTank && !botIsTank)
            return false;

        if (memberIsTank == botIsTank && member->GetExactDist2d(kologarn) < myDistance)
            return false;
    }

    return true;
}

bool KologarnFallFromFloorTrigger::IsActive()
{
    if (!GetKologarn(botAI))
        return false;

    return bot->GetPositionZ() < ULDUAR_KOLOGARN_AXIS_Z_PATHING_ISSUE_DETECT;
}

bool KologarnRubbleSlowdownTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI))
        return false;

    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (bot->HasSpellCooldown(SPELL_FROST_TRAP))
        return false;

    return GetKologarnNearestRubble(botAI, bot) != nullptr;
}

bool KologarnEyebeamTrigger::IsActive()
{
    if (!KologarnEncounterActive(botAI) || !bot->IsAlive())
        return false;

    // The chased bot has to run whatever the range: the eye follows and will close on its own.
    if (GetKologarnEyebeamChasing(botAI, bot))
        return true;

    return GetKologarnNearestEyebeam(botAI, bot, ULDUAR_KOLOGARN_EYEBEAM_REACT_RADIUS) != nullptr;
}
