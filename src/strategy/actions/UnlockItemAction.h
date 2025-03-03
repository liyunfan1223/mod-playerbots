#ifndef _PLAYERBOT_UNLOCKITEMACTION_H
#define _PLAYERBOT_UNLOCKITEMACTION_H

#include "GenericSpellActions.h"

class PlayerbotAI;

class UnlockItemAction : public CastSpellAction
{
public:
    UnlockItemAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "unlock item") { }

    bool Execute(Event event) override;

private:
    bool CanUnlockItem(Item* item);
    void UnlockItem(Item* item, uint8 bag, uint8 slot);
};

#endif
