#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERS_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERS_H

#include "Trigger.h"

class KarazhanAttumenTheHuntsmanTrigger : public Trigger
{
public:
    KarazhanAttumenTheHuntsmanTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan attumen the huntsman") {}
    bool IsActive() override;
};

class KarazhanMoroesTrigger : public Trigger
{
public:
    KarazhanMoroesTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan moroes") {}
    bool IsActive() override;
};

class KarazhanMaidenOfVirtueTrigger : public Trigger
{
public:
    KarazhanMaidenOfVirtueTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan maiden of virtue") {}
    bool IsActive() override;
};

class KarazhanBigBadWolfTrigger : public Trigger
{
public:
    KarazhanBigBadWolfTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan big bad wolf") {}
    bool IsActive() override;
};

class KarazhanRomuloAndJulianneTrigger : public Trigger
{
public:
    KarazhanRomuloAndJulianneTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan romulo and julianne") {}
    bool IsActive() override;
};

class KarazhanWizardOfOzTrigger : public Trigger
{
public:
    KarazhanWizardOfOzTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan wizard of oz") {}
    bool IsActive() override;
};

class KarazhanTheCuratorTrigger : public Trigger
{
public:
    KarazhanTheCuratorTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan the curator") {}
    bool IsActive() override;
};

class KarazhanTerestianIllhoofTrigger : public Trigger
{
public:
    KarazhanTerestianIllhoofTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan terestian illhoof") {}
    bool IsActive() override;
};

class KarazhanShadeOfAranTrigger : public Trigger
{
public:
    KarazhanShadeOfAranTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan shade of aran") {}
    bool IsActive() override;
};

class KarazhanNetherspiteTrigger : public Trigger
{
public:
    KarazhanNetherspiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan netherspite") {}
    bool IsActive() override;
};

class KarazhanPrinceMalchezaarTrigger : public Trigger
{
public:
    KarazhanPrinceMalchezaarTrigger(PlayerbotAI* botAI) : Trigger(botAI, "karazhan prince malchezaar") {}
    bool IsActive() override;
};

#endif
