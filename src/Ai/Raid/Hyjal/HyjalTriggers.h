/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALTRIGGERS_H
#define PLAYERBOTS_HYJALTRIGGERS_H

#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Trigger.h"
#include <string>

// General

class HyjalEncounterTrigger : public Trigger
{
public:
    HyjalEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, HyjalHelpers::HYJAL_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

class HyjalNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    HyjalNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "hyjal no encounter in progress", 1000) {}
    bool IsActive() override;
};

// For Misdirection to the boss. Anetheron is not included because Misdirection is used there for
// picking up Infernals as well.
class HyjalPullingBossTrigger : public HyjalEncounterTrigger
{
public:
    HyjalPullingBossTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : HyjalEncounterTrigger(botAI, name), _bossName(bossName) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
};

// This covers all five boss tanking actions, and activeAboveHealthPct is used for Archimonde
// only. Anetheron, Kaz'rogal, and Azgalor need their offtanks free for the Infernals, the
// Malevolent Cleave split, and the Doomguards, respectively, so those three are main tank only.
class HyjalBossShouldBeTankedTrigger : public HyjalEncounterTrigger
{
public:
    HyjalBossShouldBeTankedTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName,
        float activeAboveHealthPct = 0.0f, bool mainTankOnly = true)
        : HyjalEncounterTrigger(botAI, name), _bossName(bossName),
          _activeAboveHealthPct(activeAboveHealthPct), _mainTankOnly(mainTankOnly) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
    float const _activeAboveHealthPct;
    bool const _mainTankOnly;
};

// Rage Winterchill

class RageWinterchillRangedShouldSpreadTrigger : public HyjalEncounterTrigger
{
public:
    RageWinterchillRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "rage winterchill ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class RageWinterchillMeleeNearDeathAndDecayTrigger : public HyjalEncounterTrigger
{
public:
    RageWinterchillMeleeNearDeathAndDecayTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "rage winterchill melee near death and decay") {}

protected:
    bool IsActiveInEncounter() override;
};

class RageWinterchillRangedInDeathAndDecayTrigger : public HyjalEncounterTrigger
{
public:
    RageWinterchillRangedInDeathAndDecayTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "rage winterchill ranged in death and decay") {}

protected:
    bool IsActiveInEncounter() override;
};

// Anetheron

class AnetheronPullingBossOrInfernalTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronPullingBossOrInfernalTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron pulling boss or infernal") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronRangedShouldSpreadTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronNearInfernoTargetTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronNearInfernoTargetTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron near inferno target") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronTargetedByInfernalTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronTargetedByInfernalTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron targeted by infernal") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronInfernalsPulseImmolationTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronInfernalsPulseImmolationTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron infernals pulse immolation") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronInfernalsShouldBeTankedAwayTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronInfernalsShouldBeTankedAwayTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron infernals should be tanked away") {}

protected:
    bool IsActiveInEncounter() override;
};

class AnetheronShouldDivideDpsTrigger : public HyjalEncounterTrigger
{
public:
    AnetheronShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "anetheron should divide dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Kaz'rogal

class KazrogalCanSplitMalevolentCleaveDamageTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalCanSplitMalevolentCleaveDamageTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal can split malevolent cleave damage") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalRangedShouldAvoidWarStompTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalRangedShouldAvoidWarStompTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal ranged should avoid war stomp") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalLowOnManaTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalLowOnManaTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal low on mana") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalHunterShouldPreserveManaTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalHunterShouldPreserveManaTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal hunter should preserve mana") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalMarkOnMageOrPaladinTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalMarkOnMageOrPaladinTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal mark on mage or paladin") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalImmunityNoLongerNeededTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalImmunityNoLongerNeededTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal immunity no longer needed") {}

protected:
    bool IsActiveInEncounter() override;
};

class KazrogalWarlockShouldManageManaTrigger : public HyjalEncounterTrigger
{
public:
    KazrogalWarlockShouldManageManaTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "kaz'rogal warlock should manage mana") {}

protected:
    bool IsActiveInEncounter() override;
};

// Azgalor

class AzgalorRangedShouldSpreadTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorMeleeNearRainOfFireTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorMeleeNearRainOfFireTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor melee near rain of fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorRangedInRainOfFireTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorRangedInRainOfFireTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor ranged in rain of fire") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorBotIsDoomedTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorBotIsDoomedTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor bot is doomed") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorShouldControlDoomguardsTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorShouldControlDoomguardsTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor should control doomguards") {}

protected:
    bool IsActiveInEncounter() override;
};

class AzgalorShouldDivideDpsTrigger : public HyjalEncounterTrigger
{
public:
    AzgalorShouldDivideDpsTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "azgalor should divide dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Archimonde

class ArchimondeShamanShouldProtectAgainstFearTrigger : public HyjalEncounterTrigger
{
public:
    ArchimondeShamanShouldProtectAgainstFearTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "archimonde shaman should protect against fear") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeCastingAirBurstTrigger : public HyjalEncounterTrigger
{
public:
    ArchimondeCastingAirBurstTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "archimonde casting air burst") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeRangedShouldSpreadTrigger : public HyjalEncounterTrigger
{
public:
    ArchimondeRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "archimonde ranged should spread") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeNearDoomfireTrigger : public HyjalEncounterTrigger
{
public:
    ArchimondeNearDoomfireTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "archimonde near doomfire") {}

protected:
    bool IsActiveInEncounter() override;
};

class ArchimondeStoodInDoomfireTrigger : public HyjalEncounterTrigger
{
public:
    ArchimondeStoodInDoomfireTrigger(PlayerbotAI* botAI)
        : HyjalEncounterTrigger(botAI, "archimonde stood in doomfire") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
