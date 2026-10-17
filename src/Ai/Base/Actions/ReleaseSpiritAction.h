/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RELEASESPIRITACTION_H
#define PLAYERBOTS_RELEASESPIRITACTION_H

#include "Action.h"
#include "ReviveFromCorpseAction.h"

class PlayerbotAI;

class ReleaseSpiritAction : public Action
{
public:
    ReleaseSpiritAction(PlayerbotAI* botAI, std::string const& name = "release")
        : Action(botAI, name) {}

    bool Execute(Event event) override;
    void LogRelease(std::string const& releaseType) const;

protected:
    void IncrementDeathCount() const;
};

class AutoReleaseSpiritAction : public ReleaseSpiritAction
{
public:
    AutoReleaseSpiritAction(PlayerbotAI* botAI, std::string const& name = "auto release")
        : ReleaseSpiritAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool HandleBattlegroundSpiritHealer();
    bool ShouldAutoRelease() const;
    bool ShouldDelayBattlegroundRelease() const;

    time_t m_bgGossipTime = 0;
};

class RepopAction : public SpiritHealerAction
{
public:
    RepopAction(PlayerbotAI* botAI, std::string const& name = "repop")
        : SpiritHealerAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    int64 CalculateDeadTime() const;
    void PerformGraveyardTeleport(GraveyardStruct const* graveyard) const;
};

// SelfResurrectAction action registration
class SelfResurrectAction : public Action
{
public:
    SelfResurrectAction(PlayerbotAI* botAI) : Action(botAI, "self resurrect") {}
    virtual bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
