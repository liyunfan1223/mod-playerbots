/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDREDIRECTTHREAT_H
#define PLAYERBOTS_RAIDREDIRECTTHREAT_H

#include "Action.h"
#include "AttackAction.h"
#include "Define.h"

class Player;
class PlayerbotAI;
class Unit;

// Left on the hunter by Misdirection (34477) and used up by the next 3 shots
constexpr uint32 SPELL_MISDIRECTION_PROC = 35079;

// Misdirection and Tricks of the Trade. An encounter only says which tank should own the threat and
// which unit the charges are spent on.
class RaidRedirectThreatAction : public AttackAction
{
public:
    RaidRedirectThreatAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    // nullptr means nothing to redirect this tick
    virtual Player* GetRedirectTank() = 0;
    virtual Unit* GetThreatDumpTarget() = 0;

    // Group tank target is attacking, if any
    Player* GetTankHolding(Unit* target);
};

#endif
