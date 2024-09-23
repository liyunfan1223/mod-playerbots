#ifndef _PLAYERBOT_RAIDICCACTIONS_H
#define _PLAYERBOT_RAIDICCACTIONS_H

#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class IccCannonFireAction : public Action
{
public:
    IccCannonFireAction(PlayerbotAI* botAI, std::string const name = "icc cannon fire")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class IccGunshipEnterCannonAction : public MovementAction
{
public:
    IccGunshipEnterCannonAction(PlayerbotAI* botAI, std::string const name = "icc gunship enter cannon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

#endif
