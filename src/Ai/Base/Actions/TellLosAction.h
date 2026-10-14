/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLLOSACTION_H
#define PLAYERBOTS_TELLLOSACTION_H

#include "Action.h"

class PlayerbotAI;

class TellLosAction : public Action
{
public:
    TellLosAction(PlayerbotAI* botAI) : Action(botAI, "los") {}

    bool Execute(Event event) override;

private:
    void ListUnits(std::string const title, GuidVector units);
    void ListGameObjects(std::string const title, GuidVector gos);
};

class TellAuraAction : public Action
{
public:
    TellAuraAction(PlayerbotAI* botAI) : Action(botAI, "aura") {}

    virtual bool Execute(Event event);
};

class TellEstimatedDpsAction : public Action
{
public:
    TellEstimatedDpsAction(PlayerbotAI* botAI) : Action(botAI, "tell estimated dps") {}

    virtual bool Execute(Event event);
};

class TellCalculateItemAction : public Action
{
public:
    TellCalculateItemAction(PlayerbotAI* botAI) : Action(botAI, "calculate item") {}

    virtual bool Execute(Event event);
};

#endif
