/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BWLTRIGGERS_H
#define PLAYERBOTS_BWLTRIGGERS_H

#include "Trigger.h"

// General

class BwlSuppressionDeviceTrigger : public Trigger
{
public:
    BwlSuppressionDeviceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl suppression device") {}
    bool IsActive() override;
};

// Razorgore the Untamed

class BwlRazorgoreNotMindControlledTrigger : public Trigger
{
public:
    BwlRazorgoreNotMindControlledTrigger(PlayerbotAI* botAI, std::string const& name = "bwl razorgore not mind controlled") : Trigger(botAI, name) {}
    bool IsActive() override;
};

// Vaelastrasz the Corrupt

class BwlVaelastraszPositioningTrigger : public Trigger
{
public:
    BwlVaelastraszPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl vaelastrasz positioning") {}
    bool IsActive() override;
};

class BwlVaelastraszBurningAdrenalineTrigger : public Trigger
{
public:
    BwlVaelastraszBurningAdrenalineTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl vaelastrasz burning adrenaline") {}
    bool IsActive() override;
};

// Broodlord Lashlayer

class BwlBroodlordRangedTooCloseTrigger : public Trigger
{
public:
    BwlBroodlordRangedTooCloseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl broodlord ranged too close") {}
    bool IsActive() override;
};

// Firemaw / Ebonroc / Flamegor

class BwlBlackDrakeNotVictimTrigger : public Trigger
{
public:
    BwlBlackDrakeNotVictimTrigger(PlayerbotAI* botAI, std::string bossName)
        : Trigger(botAI, "bwl " + bossName + " not victim"),
          bossName(std::move(bossName)) {}
    bool IsActive() override;

private:
    const std::string bossName;
};

// Chromaggus

class BwlAfflictionBronzeTrigger : public Trigger
{
public:
    BwlAfflictionBronzeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl affliction bronze") {}
    bool IsActive() override;
};

// Nefarian

class BwlNefarianPositioningTrigger : public Trigger
{
public:
    BwlNefarianPositioningTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl nefarian positioning") {}
    bool IsActive() override;
};

class BwlNefarianWildMagicTrigger : public Trigger
{
public:
    BwlNefarianWildMagicTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl nefarian wild magic") {}
    bool IsActive() override;
};

// Trash

class BwlDeathTalonWyrmguardTankTrigger : public Trigger
{
public:
    BwlDeathTalonWyrmguardTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl death talon wyrmguard tank") {}
    bool IsActive() override;
};

class BwlDeathTalonWyrmguardRangedTrigger : public Trigger
{
public:
    BwlDeathTalonWyrmguardRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "bwl death talon wyrmguard ranged") {}
    bool IsActive() override;
};

#endif
