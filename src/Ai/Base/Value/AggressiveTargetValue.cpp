/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AggressiveTargetValue.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"

Unit* AggressiveTargetValue::Calculate()
{
    Player* master = GetMaster();

    if (master && (master == bot || master->GetMapId() != bot->GetMapId() || master->IsBeingTeleported() ||
                   !GET_PLAYERBOT_AI(master)))
        master = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    if (targets.empty())
        return nullptr;

    float aggroRange = sPlayerbotAIConfig.AggroDistance;
    float distance = 0;
    Unit* result = nullptr;

    for (ObjectGuid const guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (!unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
            continue;

        if (unit->ToCreature() && !unit->ToCreature()->GetCreatureTemplate()->lootid &&
            bot->GetReactionTo(unit) >= REP_NEUTRAL)
            continue;

        if (!bot->IsHostileTo(unit) && unit->GetNpcFlags() != UNIT_NPC_FLAG_NONE)
            continue;

        if (abs(bot->GetPositionZ() - unit->GetPositionZ()) > INTERACTION_DISTANCE)
            continue;

        if (!bot->InBattleground() && master && botAI->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) &&
            ServerFacade::instance().GetDistance2d(master, unit) > aggroRange)
            continue;

        if (!bot->IsWithinLOSInMap(unit))
            continue;

        if (bot->GetDistance(unit) > aggroRange)
            continue;

        float newdistance = bot->GetDistance(unit);
        if (!result || (newdistance < distance))
        {
            distance = newdistance;
            result = unit;
        }
    }

    return result;
}
