/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDANTIFEAR_H
#define PLAYERBOTS_RAIDANTIFEAR_H

#include "Action.h"
#include "Define.h"
#include "Multiplier.h"
#include "Trigger.h"

class Player;
class PlayerbotAI;

constexpr uint32 SPELL_FEAR_WARD = 6346;

// The main tank, else the first healer in range without the ward. nullptr when nobody needs it or Fear
// Ward is on cooldown.
Player* GetAntiFearWardTarget(PlayerbotAI* botAI, Player* bot);

// Shaman knows Tremor Totem, doesn't have one out and can cast it now
bool ShouldDropTremorTotem(PlayerbotAI* botAI, Player* bot);

bool RaidAntiFearReady(PlayerbotAI* botAI, Player* bot);

// Fear Ward and Tremor Totem for a boss fear. An encounter subclasses these and supplies the fear window.
// Tremor is cast directly since "+tremor" never wins the earth slot, and the multiplier holds the others.
class RaidAntiFearTrigger : public Trigger
{
public:
    RaidAntiFearTrigger(PlayerbotAI* botAI, std::string const name) : Trigger(botAI, name) {}
    bool IsActive() override;

protected:
    virtual bool FearWindowActive() = 0;
};

class RaidAntiFearAction : public Action
{
public:
    RaidAntiFearAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    virtual bool FearWindowActive() = 0;
};

class RaidAntiFearTotemGuardMultiplier : public Multiplier
{
public:
    RaidAntiFearTotemGuardMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}
    float GetValue(Action* action) override;

protected:
    virtual bool FearWindowActive() = 0;
};

#endif
