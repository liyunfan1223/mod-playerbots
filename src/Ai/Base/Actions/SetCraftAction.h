/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETCRAFTACTION_H
#define PLAYERBOTS_SETCRAFTACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

class SetCraftAction : public Action
{
public:
    SetCraftAction(PlayerbotAI* botAI) : Action(botAI, "craft") {}

    bool Execute(Event event) override;

private:
    void TellProfessions(Player* requester);
    bool PlaceOrder(std::string const& param);
    uint32 FindCraftSpell(uint32 itemId);
};

#endif
