/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_READYCHECKACTION_H
#define PLAYERBOTS_READYCHECKACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class ReadyCheckAction : public InventoryAction
{
public:
    ReadyCheckAction(PlayerbotAI* botAI, std::string const name = "ready check") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    bool ReadyCheck();
    void ReportReadinessToMaster();
};

class FinishReadyCheckAction : public ReadyCheckAction
{
public:
    FinishReadyCheckAction(PlayerbotAI* botAI) : ReadyCheckAction(botAI, "finish ready check") {}

    bool Execute(Event event) override;
};

class ForceRebuffAction : public Action
{
public:
    ForceRebuffAction(PlayerbotAI* botAI) : Action(botAI, "force rebuff") {}

    bool Execute(Event event) override;
};

class ReadyReplyAction : public Action
{
public:
    ReadyReplyAction(PlayerbotAI* botAI) : Action(botAI, "ready reply") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
