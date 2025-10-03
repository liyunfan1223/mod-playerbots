#ifndef _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H

#include "Trigger.h"

class HighKingMaulgarTrigger : public Trigger
{
public:
    HighKingMaulgarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar") {}
    bool IsActive() override;
};

class GruulTheDragonkillerTrigger : public Trigger
{
public:
    GruulTheDragonkillerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller") {}
    bool IsActive() override;
};

#endif
