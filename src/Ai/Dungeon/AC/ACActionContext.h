/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACACTIONCONTEXT_H
#define PLAYERBOTS_ACACTIONCONTEXT_H

#include "ACActions.h"
#include "NamedObjectContext.h"

class TbcDungeonAuchenaiCryptsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonAuchenaiCryptsActionContext()
    {
        creators["shirrak tank position boss"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_tank_position_boss;

        creators["shirrak flee focus fire"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_flee_focus_fire;

        creators["shirrak ranged keep distance"] =
            &TbcDungeonAuchenaiCryptsActionContext::shirrak_ranged_keep_distance;
    }
private:
    static Action* shirrak_tank_position_boss(
        PlayerbotAI* botAI) { return new ShirrakTankPositionBossAction(botAI); }

    static Action* shirrak_flee_focus_fire(
        PlayerbotAI* botAI) { return new ShirrakFleeFocusFireAction(botAI); }

    static Action* shirrak_ranged_keep_distance(
        PlayerbotAI* botAI) { return new ShirrakRangedKeepDistanceAction(botAI); }
};

#endif
