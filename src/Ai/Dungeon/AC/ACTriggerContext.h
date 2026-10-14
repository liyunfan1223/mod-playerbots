/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACTRIGGERCONTEXT_H
#define PLAYERBOTS_ACTRIGGERCONTEXT_H

#include "ACTriggers.h"
#include "NamedObjectContext.h"

class TbcDungeonAuchenaiCryptsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    // Shirrak the Dead Watcher
    TbcDungeonAuchenaiCryptsTriggerContext()
    {
        creators["shirrak tank position boss"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_tank_position_boss;

        creators["shirrak flee focus fire"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_flee_focus_fire;

        creators["shirrak ranged keep distance"] =
            &TbcDungeonAuchenaiCryptsTriggerContext::shirrak_ranged_keep_distance;
    }
private:
    // Shirrak the Dead Watcher
    static Trigger* shirrak_tank_position_boss(
        PlayerbotAI* botAI) { return new ShirrakTankPositionBossTrigger(botAI); }

    static Trigger* shirrak_flee_focus_fire(
        PlayerbotAI* botAI) { return new ShirrakFleeFocusFireTrigger(botAI); }

    static Trigger* shirrak_ranged_keep_distance(
        PlayerbotAI* botAI) { return new ShirrakRangedKeepDistanceTrigger(botAI); }
};

#endif
