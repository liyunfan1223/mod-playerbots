#ifndef _PLAYERBOT_RAIDAQ20ACTIONS_H
#define _PLAYERBOT_RAIDAQ20ACTIONS_H

#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class Aq20UseCrystalAction : public MovementAction
{
public:
    Aq20UseCrystalAction(PlayerbotAI* botAI, std::string const name = "aq20 use crystal") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
#endif
