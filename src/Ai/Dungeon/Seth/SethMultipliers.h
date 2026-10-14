/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHMULTIPLIERS_H
#define PLAYERBOTS_SETHMULTIPLIERS_H

#include "Multiplier.h"

class SethekkProphetSetTremorTotemMultiplier : public Multiplier
{
public:
    SethekkProphetSetTremorTotemMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "sethekk prophet set tremor totem") {}
    float GetValue(Action* action) override;
};

class AnzuControlSpellCastingWithSpellBombMultiplier : public Multiplier
{
public:
    AnzuControlSpellCastingWithSpellBombMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anzu control spell casting with spell bomb") {}
    float GetValue(Action* action) override;
};

class TalonKingIkissDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    TalonKingIkissDelayBloodlustAndHeroismMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "talon king ikiss delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class TalonKingIkissControlMovementMultiplier : public Multiplier
{
public:
    TalonKingIkissControlMovementMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "talon king ikiss control movement") {}
    float GetValue(Action* action) override;
};

#endif
