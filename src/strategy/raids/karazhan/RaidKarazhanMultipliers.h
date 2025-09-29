#ifndef _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H
#define _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H

#include "Multiplier.h"

class KarazhanAttumenTheHuntsmanMultiplier : public Multiplier
{
public:
    KarazhanAttumenTheHuntsmanMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan attumen the huntsman multiplier") {}
    virtual float GetValue(Action* action);
};

class KarazhanBigBadWolfMultiplier : public Multiplier
{
public:
    KarazhanBigBadWolfMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan big bad wolf multiplier") {}
    virtual float GetValue(Action* action);
};

class KarazhanShadeOfAranMultiplier : public Multiplier
{
public:
    KarazhanShadeOfAranMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan shade of aran multiplier") {}
    virtual float GetValue(Action* action);
};

class KarazhanNetherspiteBlueAndGreenBeamMultiplier : public Multiplier
{
public:
    KarazhanNetherspiteBlueAndGreenBeamMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan netherspite blue and green beam multiplier") {}
    virtual float GetValue(Action* action);
};

class KarazhanNetherspiteRedBeamMultiplier : public Multiplier
{
public:
    KarazhanNetherspiteRedBeamMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan netherspite red beam multiplier") {}
    virtual float GetValue(Action* action);
};

class KarazhanPrinceMalchezaarMultiplier : public Multiplier
{
public:
    KarazhanPrinceMalchezaarMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "karazhan prince malchezaar multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
