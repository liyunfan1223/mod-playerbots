/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldActions_Kologarn.h"
#include "UldActions_Shared.h"

#include <cmath>

#include "AiObjectContext.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Position.h"
#include "UldEncounter_Kologarn.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "ServerFacade.h"
#include "Unit.h"

const Position ULDUAR_KOLOGARN_RESTORE_POSITION = Position(1764.3749f, -24.02903f, 448.0f, 0.00087690353f);

bool KologarnBodyTankAction::isUseful()
{
    KologarnBodyTankTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnBodyTankAction::Execute(Event /*event*/)
{
    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return false;

    if (AI_VALUE(Unit*, "current target") != kologarn)
        return Attack(kologarn);

    // Petrifying Breath goes out the moment the body's victim is out of melee range.
    if (!bot->IsWithinMeleeRange(kologarn))
        return MoveTo(kologarn, 0.0f, MovementPriority::MOVEMENT_COMBAT);

    return false;
}

bool KologarnOffTankAction::isUseful()
{
    KologarnOffTankTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnOffTankAction::Execute(Event /*event*/)
{
    Unit* target = GetKologarnOffTankTarget(botAI, bot);
    if (!target)
        return false;

    return Attack(target);
}

bool KologarnRubbleTankAction::isUseful()
{
    KologarnRubbleTankTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnRubbleTankAction::Execute(Event event)
{
    Unit* rubble = GetKologarnLooseRubble(botAI, bot);
    if (!rubble)
        return false;

    if (AI_VALUE(Unit*, "current target") != rubble)
        return Attack(rubble);

    if (rubble->GetVictim() != bot && botAI->DoSpecificAction("taunt spell", event, true))
        return true;

    // Held, not kited: rubble run 8.0 yd/s against a player's 7.0.
    Position const hold = GetKologarnRubbleHoldSpot(botAI, rubble);
    if (bot->GetExactDist2d(hold.GetPositionX(), hold.GetPositionY()) < sPlayerbotAIConfig.FollowDistance)
        return false;

    return MoveTo(bot->GetMapId(), hold.GetPositionX(), hold.GetPositionY(), hold.GetPositionZ(), false, false, false,
                  false, MovementPriority::MOVEMENT_COMBAT);
}

bool KologarnDpsTargetAction::isUseful()
{
    KologarnDpsTargetTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnDpsTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetKologarnDpsTarget(botAI, bot);
    if (!target)
        return false;

    return Attack(target);
}

bool KologarnSmashSwapAction::isUseful()
{
    KologarnSmashSwapTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnSmashSwapAction::Execute(Event event)
{
    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return false;

    if (AI_VALUE(Unit*, "current target") != kologarn)
        return Attack(kologarn);

    if (kologarn->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    return false;
}

bool KologarnBodyUncoveredAction::isUseful()
{
    KologarnBodyUncoveredTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnBodyUncoveredAction::Execute(Event /*event*/)
{
    Unit* kologarn = GetKologarn(botAI);
    if (!kologarn)
        return false;

    if (AI_VALUE(Unit*, "current target") != kologarn)
        return Attack(kologarn);

    return MoveTo(kologarn, 0.0f, MovementPriority::MOVEMENT_COMBAT);
}

bool KologarnFallFromFloorAction::isUseful()
{
    KologarnFallFromFloorTrigger trigger(botAI);
    return trigger.IsActive();
}

// Pathing workaround, not a mechanic: a bot under the walkway is inside the pit bunny's kill box and
// dies within a second, so there is no walk-back to attempt.
bool KologarnFallFromFloorAction::Execute(Event /*event*/)
{
    bot->NearTeleportTo(ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionX(),
                        ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionY(),
                        ULDUAR_KOLOGARN_RESTORE_POSITION.GetPositionZ(),
                        ULDUAR_KOLOGARN_RESTORE_POSITION.GetOrientation());
    return true;
}

bool KologarnRubbleSlowdownAction::Execute(Event /*event*/)
{
    Unit* rubble = GetKologarnNearestRubble(botAI, bot);
    if (!rubble)
        return false;

    return botAI->CastSpell("frost trap", rubble);
}

bool KologarnEyebeamAction::isUseful()
{
    KologarnEyebeamTrigger trigger(botAI);
    return trigger.IsActive();
}

bool KologarnEyebeamAction::Execute(Event /*event*/)
{
    // The chased bot outruns the eye (5.5 yd/s against 7.0) only while it keeps moving. Everyone else
    // just clears the 3 yd beam.
    if (Unit* chasing = GetKologarnEyebeamChasing(botAI, bot))
    {
        Position const step = GetKologarnEyebeamEscapeStep(bot, chasing);
        return MoveTo(bot->GetMapId(), step.GetPositionX(), step.GetPositionY(), step.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED);
    }

    Unit* eye = GetKologarnNearestEyebeam(botAI, bot, ULDUAR_KOLOGARN_EYEBEAM_REACT_RADIUS);
    if (!eye)
        return false;

    return FleePosition(eye->GetPosition(), ULDUAR_KOLOGARN_EYEBEAM_SAFE_DISTANCE);
}
