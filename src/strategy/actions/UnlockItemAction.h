#ifndef _PLAYERBOT_UNLOCKITEMACTION_H
#define _PLAYERBOT_UNLOCKITEMACTION_H

#include "Action.h"

class PlayerbotAI;

class UnlockItemAction : public Action
{
public:
    UnlockItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock item") { }

    bool Execute(Event event) override;

private:
    void UnlockItem(Item* item);
};

#endif
