/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPLOADOUTACTION_H
#define PLAYERBOTS_PVPLOADOUTACTION_H

#include "Action.h"

class PlayerbotAI;

// Enters, re-applies or restores the PvP loadout. Entering plans the gear and saves copies of the PvE items it
// replaces; a later pass equips it once they are saved.
class SwapPvpLoadoutAction : public Action
{
public:
    SwapPvpLoadoutAction(PlayerbotAI* botAI) : Action(botAI, "swap pvp loadout") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;

private:
    void Wear(uint32 rating);
};

#endif
