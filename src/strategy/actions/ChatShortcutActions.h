/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CHATSHORTCUTACTION_H
#define _PLAYERBOT_CHATSHORTCUTACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class ReturnPositionResetAction : public Action
{
    public:
        ReturnPositionResetAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) { }

        void ResetReturnPosition();
        void SetReturnPosition(float x, float y, float z);
};

class FollowChatShortcutAction : public MovementAction
{
    public:
        FollowChatShortcutAction(PlayerbotAI* botAI) : MovementAction(botAI, "follow chat shortcut") { }

        bool Execute(Event event) override;
};

class StayChatShortcutAction : public ReturnPositionResetAction
{
    public:
        StayChatShortcutAction(PlayerbotAI* botAI) : ReturnPositionResetAction(botAI, "stay chat shortcut") { }

        bool Execute(Event event) override;
};

class FleeChatShortcutAction : public ReturnPositionResetAction
{
    public:
        FleeChatShortcutAction(PlayerbotAI* botAI) : ReturnPositionResetAction(botAI, "flee chat shortcut") { }

        bool Execute(Event event) override;
};

class GoawayChatShortcutAction : public ReturnPositionResetAction
{
    public:
        GoawayChatShortcutAction(PlayerbotAI* botAI) : ReturnPositionResetAction(botAI, "runaway chat shortcut") { }

        bool Execute(Event event) override;
};

class GrindChatShortcutAction : public ReturnPositionResetAction
{
    public:
        GrindChatShortcutAction(PlayerbotAI* botAI) : ReturnPositionResetAction(botAI, "grind chat shortcut") { }

        bool Execute(Event event) override;
};

class TankAttackChatShortcutAction : public ReturnPositionResetAction
{
    public:
        TankAttackChatShortcutAction(PlayerbotAI* botAI) : ReturnPositionResetAction(botAI, "tank attack chat shortcut") { }

        bool Execute(Event event) override;
};

class MaxDpsChatShortcutAction : public Action
{
    public:
        MaxDpsChatShortcutAction(PlayerbotAI* botAI) : Action(botAI, "max dps chat shortcut") { }

        bool Execute(Event event) override;
};

class NaxxChatShortcutAction : public Action
{
    public:
        NaxxChatShortcutAction(PlayerbotAI* ai) : Action(ai, "naxx chat shortcut") {}
        virtual bool Execute(Event event);
};

class BwlChatShortcutAction : public Action
{
    public:
        BwlChatShortcutAction(PlayerbotAI* ai) : Action(ai, "bwl chat shortcut") {}
        virtual bool Execute(Event event);
};
#endif
