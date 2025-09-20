#ifndef _PLAYERBOT_RAIDGRUULSLAIRTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRTRIGGERCONTEXT_H

#include "RaidGruulsLairTriggers.h"
#include "AiObjectContext.h"

class RaidGruulsLairTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidGruulsLairTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["high king maulgar"] = &RaidGruulsLairTriggerContext::high_king_maulgar;
        creators["gruul the dragonkiller"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller;
    }

private:
    static Trigger* high_king_maulgar(PlayerbotAI* botAI) { return new HighKingMaulgarTrigger(botAI); }
    static Trigger* gruul_the_dragonkiller(PlayerbotAI* botAI) { return new GruulTheDragonkillerTrigger(botAI); }
};

#endif
