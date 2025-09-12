#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERCONTEXT_H

#include "RaidKarazhanTriggers.h"
#include "AiObjectContext.h"

class RaidKarazhanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidKarazhanTriggerContext()
    {
        creators["karazhan attumen the huntsman"] = &RaidKarazhanTriggerContext::karazhan_attumen_the_huntsman;
        creators["karazhan moroes"] = &RaidKarazhanTriggerContext::karazhan_moroes;
        creators["karazhan maiden of virtue"] = &RaidKarazhanTriggerContext::karazhan_maiden_of_virtue;
        creators["karazhan big bad wolf"] = &RaidKarazhanTriggerContext::karazhan_big_bad_wolf;
        creators["karazhan romulo and julianne"] = &RaidKarazhanTriggerContext::karazhan_romulo_and_julianne;
        creators["karazhan wizard of oz"] = &RaidKarazhanTriggerContext::karazhan_wizard_of_oz;
        creators["karazhan the curator"] = &RaidKarazhanTriggerContext::karazhan_the_curator;
        creators["karazhan terestian illhoof"] = &RaidKarazhanTriggerContext::karazhan_terestian_illhoof;
        creators["karazhan shade of aran"] = &RaidKarazhanTriggerContext::karazhan_shade_of_aran;
        creators["karazhan netherspite"] = &RaidKarazhanTriggerContext::karazhan_netherspite;
        creators["karazhan prince malchezaar"] = &RaidKarazhanTriggerContext::karazhan_prince_malchezaar;
    }

private:
    static Trigger* karazhan_attumen_the_huntsman(PlayerbotAI* botAI) { return new KarazhanAttumenTheHuntsmanTrigger(botAI); }
    static Trigger* karazhan_moroes(PlayerbotAI* botAI) { return new KarazhanMoroesTrigger(botAI); }
    static Trigger* karazhan_maiden_of_virtue(PlayerbotAI* botAI) { return new KarazhanMaidenOfVirtueTrigger(botAI); }
    static Trigger* karazhan_big_bad_wolf(PlayerbotAI* botAI) { return new KarazhanBigBadWolfTrigger(botAI); }
    static Trigger* karazhan_romulo_and_julianne(PlayerbotAI* botAI) { return new KarazhanRomuloAndJulianneTrigger(botAI); }
    static Trigger* karazhan_wizard_of_oz(PlayerbotAI* botAI) { return new KarazhanWizardOfOzTrigger(botAI); }
    static Trigger* karazhan_the_curator(PlayerbotAI* botAI) { return new KarazhanTheCuratorTrigger(botAI); }
    static Trigger* karazhan_terestian_illhoof(PlayerbotAI* botAI) { return new KarazhanTerestianIllhoofTrigger(botAI); }
    static Trigger* karazhan_shade_of_aran(PlayerbotAI* botAI) { return new KarazhanShadeOfAranTrigger(botAI); }
    static Trigger* karazhan_netherspite(PlayerbotAI* botAI) { return new KarazhanNetherspiteTrigger(botAI); }
    static Trigger* karazhan_prince_malchezaar(PlayerbotAI* botAI) { return new KarazhanPrinceMalchezaarTrigger(botAI); }
};

#endif
