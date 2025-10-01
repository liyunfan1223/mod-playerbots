/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOT_RELEASESPIRITACTION_H
#define PLAYERBOT_RELEASESPIRITACTION_H

#include "Action.h"
#include "ReviveFromCorpseAction.h"

class PlayerbotAI;

class ReleaseSpiritAction : public Action
{
public:
    ReleaseSpiritAction(PlayerbotAI* botAI, const std::string& name = "release")
        : Action(botAI, name) {}

    bool Execute(Event event) override;
    void LogRelease(const std::string& releaseType, bool isAutoRelease = false) const;

protected:
    void IncrementDeathCount() const;
};

class AutoReleaseSpiritAction : public ReleaseSpiritAction
{
public:
    AutoReleaseSpiritAction(PlayerbotAI* botAI, const std::string& name = "auto release")
        : ReleaseSpiritAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool HandleBattlegroundSpiritHealer();
    bool ShouldAutoRelease() const;
    bool ShouldDelayBattlegroundRelease() const;

    inline static std::unordered_map<uint32_t, time_t> m_botReleaseTimes;
    time_t m_bgGossipTime = 0;
};

class RepopAction : public SpiritHealerAction
{
public:
    RepopAction(PlayerbotAI* botAI, const std::string& name = "repop")
        : SpiritHealerAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    int64 CalculateDeadTime() const;
    void PerformGraveyardTeleport(const GraveyardStruct* graveyard) const;
};

// SelfResurrectAction action registration
class SelfResurrectAction : public Action
{
public:
    SelfResurrectAction(PlayerbotAI* ai) : Action(ai, "self resurrect") {}
    virtual bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
