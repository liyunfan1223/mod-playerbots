#ifndef _PLAYERBOT_RAIDBWLACTIONS_H
#define _PLAYERBOT_RAIDBWLACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "AttackAction.h"
#include "GenericActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class BwlOnyxiaScaleCloakAuraCheckAction : public Action
{
    public:
        BwlOnyxiaScaleCloakAuraCheckAction(PlayerbotAI* botAI) : Action(botAI, "bwl onyxia scale cloak aura check") {}
        bool Execute(Event event) override;
        bool isUseful() override;
};

class BwlTurnOffSuppressionDeviceAction : public Action
{
    public:
        BwlTurnOffSuppressionDeviceAction(PlayerbotAI* botAI) : Action(botAI, "bwl turn off suppression device") {}
        bool Execute(Event event) override;
};

#endif