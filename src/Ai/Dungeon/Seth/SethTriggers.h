/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHTRIGGERS_H
#define PLAYERBOTS_SETHTRIGGERS_H

#include "Trigger.h"

class TimeLostControllerDropsCharmingTotemTrigger : public Trigger
{
public:
    TimeLostControllerDropsCharmingTotemTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "time-lost controller drops charming totem") {}
    bool IsActive() override;
};

class SethekkProphetCastsFearTrigger : public Trigger
{
public:
    SethekkProphetCastsFearTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sethekk prophet casts fear") {}
    bool IsActive() override;
};

class DarkweaverSythBossSummonsElementalsTrigger : public Trigger
{
public:
    DarkweaverSythBossSummonsElementalsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "darkweaver syth boss summons elementals") {}
    bool IsActive() override;
};

class AnzuEncounterHasTwoPhasesTrigger : public Trigger
{
public:
    AnzuEncounterHasTwoPhasesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anzu encounter has two phases") {}
    bool IsActive() override;
};

class AnzuBirdSpiritsProvideBuffsTrigger : public Trigger
{
public:
    AnzuBirdSpiritsProvideBuffsTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "anzu bird spirits provide buffs") {}
    bool IsActive() override;
};

class TalonKingIkissShouldBeTankedTrigger : public Trigger
{
public:
    TalonKingIkissShouldBeTankedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "talon king ikiss should be tanked") {}
    bool IsActive() override;
};

class TalonKingIkissRangedShouldStayNearTankTrigger : public Trigger
{
public:
    TalonKingIkissRangedShouldStayNearTankTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "talon king ikiss ranged should stay near tank") {}
    bool IsActive() override;
};

class TalonKingIkissBossCastingArcaneExplosionTrigger : public Trigger
{
public:
    TalonKingIkissBossCastingArcaneExplosionTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "talon king ikiss boss casting arcane explosion") {}
    bool IsActive() override;
};

class TalonKingIkissOutOfLosTrigger : public Trigger
{
public:
    TalonKingIkissOutOfLosTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "talon king ikiss out of los") {}
    bool IsActive() override;
};

#endif
