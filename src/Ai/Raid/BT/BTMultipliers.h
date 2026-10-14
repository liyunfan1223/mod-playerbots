/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTMULTIPLIERS_H
#define PLAYERBOTS_BTMULTIPLIERS_H

#include "Multiplier.h"

// High Warlord Naj'entus

class BlackTempleDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    BlackTempleDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "black temple delay dps cooldowns multiplier") {}
    float GetValue(Action* action) override;
};

class HighWarlordNajentusDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    HighWarlordNajentusDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "high warlord naj'entus disable combat formation move multiplier") {}
    float GetValue(Action* action) override;
};

// Supremus

class SupremusFocusOnAvoidanceInPhase2Multiplier : public Multiplier
{
public:
    SupremusFocusOnAvoidanceInPhase2Multiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus focus on avoidance in phase 2 multiplier") {}
    float GetValue(Action* action) override;
};

class SupremusHitboxIsBuggedMultiplier : public Multiplier
{
public:
    SupremusHitboxIsBuggedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "supremus hitbox is bugged multiplier") {}
    float GetValue(Action* action) override;
};

// Teron Gorefiend

class TeronGorefiendControlMovementMultiplier : public Multiplier
{
public:
    TeronGorefiendControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend control movement multiplier") {}
    float GetValue(Action* action) override;
};

class TeronGorefiendMarkedBotOnlyMoveToDieMultiplier : public Multiplier
{
public:
    TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend marked bot only move to die multiplier") {}
    float GetValue(Action* action) override;
};

class TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier : public Multiplier
{
public:
    TeronGorefiendSpiritsAttackOnlyShadowyConstructsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend spirits attack only shadowy constructs multiplier") {}
    float GetValue(Action* action) override;
};

class TeronGorefiendDisableAttackingConstructsMultiplier : public Multiplier
{
public:
    TeronGorefiendDisableAttackingConstructsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "teron gorefiend disable attacking constructs multiplier") {}
    float GetValue(Action* action) override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilControlMovementMultiplier : public Multiplier
{
public:
    GurtoggBloodboilControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "gurtogg bloodboil control movement multiplier") {}
    float GetValue(Action* action) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsDontWasteHealingMultiplier : public Multiplier
{
public:
    ReliquaryOfSoulsDontWasteHealingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "reliquary of souls don't waste healing multiplier") {}
    float GetValue(Action* action) override;
};

// Mother Shahraz

class MotherShahrazControlMovementMultiplier : public Multiplier
{
public:
    MotherShahrazControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz control movement multiplier") {}
    float GetValue(Action* action) override;
};

class MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier : public Multiplier
{
public:
    MotherShahrazBotsWithFatalAttractionOnlyRunAwayMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "mother shahraz bots with fatal attraction only run away multiplier") {}
    float GetValue(Action* action) override;
};

// Illidari Council

class IllidariCouncilDisableTankActionsMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class IllidariCouncilControlMovementMultiplier : public Multiplier
{
public:
    IllidariCouncilControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council control movement multiplier") {}
    float GetValue(Action* action) override;
};

class IllidariCouncilControlMisdirectionMultiplier : public Multiplier
{
public:
    IllidariCouncilControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council control misdirection multiplier") {}
    float GetValue(Action* action) override;
};

class IllidariCouncilDisableArcaneShotOnZerevorMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableArcaneShotOnZerevorMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable arcane shot on zerevor multiplier") {}
    float GetValue(Action* action) override;
};

class IllidariCouncilDisableIceBlockMultiplier : public Multiplier
{
public:
    IllidariCouncilDisableIceBlockMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council disable ice block multiplier") {}
    float GetValue(Action* action) override;
};

class IllidariCouncilWaitForDpsMultiplier : public Multiplier
{
public:
    IllidariCouncilWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidari council wait for dps multiplier") {}
    float GetValue(Action* action) override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    IllidanStormrageDelayDpsCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage delay dps cooldowns multiplier") {}
    float GetValue(Action* action) override;
};

class IllidanStormrageControlTankActionsMultiplier : public Multiplier
{
public:
    IllidanStormrageControlTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage control tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class IllidanStormrageDisableDefaultTargetingMultiplier : public Multiplier
{
public:
    IllidanStormrageDisableDefaultTargetingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage disable default targeting multiplier") {}
    float GetValue(Action* action) override;
};

class IllidanStormrageControlNonTankMovementMultiplier : public Multiplier
{
public:
    IllidanStormrageControlNonTankMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage control non-tank movement multiplier") {}
    float GetValue(Action* action) override;
};

class IllidanStormrageUseEarthbindTotemMultiplier : public Multiplier
{
public:
    IllidanStormrageUseEarthbindTotemMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage use earthbind totem multiplier") {}
    float GetValue(Action* action) override;
};

class IllidanStormrageWaitForDpsMultiplier : public Multiplier
{
public:
    IllidanStormrageWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "illidan stormrage wait for dps multiplier") {}
    float GetValue(Action* action) override;
};

#endif
