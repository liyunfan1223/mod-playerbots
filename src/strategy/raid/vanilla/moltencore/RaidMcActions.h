#ifndef _PLAYERBOT_RAIDMCACTIONS_H
#define _PLAYERBOT_RAIDMCACTIONS_H

#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class McCheckShouldMoveFromGroupAction : public Action
{
public:
    McCheckShouldMoveFromGroupAction(PlayerbotAI* botAI, std::string const name = "mc check should move from group")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class McMoveFromBaronGeddonAction : public MovementAction
{
public:
    McMoveFromBaronGeddonAction(PlayerbotAI* botAI, std::string const name = "mc move from baron geddon")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
