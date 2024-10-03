#ifndef _PLAYERBOT_OPENITEMACTION_H
#define _PLAYERBOT_OPENITEMACTION_H

#include "Action.h"
#include "Item.h"

class PlayerbotAI;

class OpenItemAction : public Action
{
public:
    OpenItemAction(PlayerbotAI* botAI) : Action(botAI, "open item") {}

    bool Execute(Event event) override;

private:
    bool CanOpenItem(Item* item);
};

#endif
