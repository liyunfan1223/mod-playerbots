/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SnareTargetValue.h"
#include "AiObjectContext.h"
#include "PlayerbotAI.h"
#include "ServerFacade.h"

Unit* SnareTargetValue::Calculate()
{
    std::string const spell = qualifier;

    GuidVector attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        if (bot->GetDistance(unit) > botAI->GetRange("spell"))
            continue;

        Unit* chaseTarget;
        switch (unit->GetMotionMaster()->GetCurrentMovementGeneratorType())
        {
            case FLEEING_MOTION_TYPE:
                return unit;
            case CHASE_MOTION_TYPE:
            {
                chaseTarget = ServerFacade::instance().GetChaseTarget(unit);
                if (!chaseTarget)
                    continue;
                Player* chaseTargetPlayer = ObjectAccessor::FindPlayer(chaseTarget->GetGUID());
                // check if need to snare
                bool shouldSnare = true;

                // do not slow down if bot is melee and mob/bot attack each other
                if (chaseTargetPlayer && !PlayerbotAI::IsRanged(bot) && chaseTargetPlayer == bot)
                    shouldSnare = false;

                if (!unit->isMoving())
                    shouldSnare = false;

                if (unit->HasAuraType(SPELL_AURA_MOD_ROOT))
                    shouldSnare = false;

                if (chaseTargetPlayer && shouldSnare && !PlayerbotAI::IsTank(chaseTargetPlayer))
                {
                    return unit;
                }
                break;
            }
            default:
                break;
        }
    }

    return nullptr;
}
