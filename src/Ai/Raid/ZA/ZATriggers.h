/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZATRIGGERS_H
#define PLAYERBOTS_ZATRIGGERS_H

#include "EncounterHelpers.h"
#include "Trigger.h"
#include "ZAHelpers.h"
#include <string>

// General

class ZulAmanEncounterTrigger : public Trigger
{
public:
    ZulAmanEncounterTrigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, ZaHelpers::ZA_MAP_ID) &&
            IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;
};

// General

class ZulAmanNoEncounterInProgressTrigger : public Trigger
{
public:
    // Throttled to once per second. This trigger is true for all trash and downtime and, being
    // for between-encounter clean-up, has no real urgency to it.
    ZulAmanNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "zul'aman no encounter in progress", 1000) {}
    bool IsActive() override;
};

// Trash

class AmanishiMedicineManSummonedWardTrigger : public Trigger
{
public:
    AmanishiMedicineManSummonedWardTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "amani'shi medicine man summoned ward") {}
    bool IsActive() override;
};

// For Misdirection to the boss on the pull. Used by every boss.
class ZulAmanPullingBossTrigger : public ZulAmanEncounterTrigger
{
public:
    ZulAmanPullingBossTrigger(
        PlayerbotAI* botAI, std::string const& name, std::string const& bossName)
        : ZulAmanEncounterTrigger(botAI, name), _bossName(bossName) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::string const _bossName;
};

// Akil'zon <Eagle Avatar>

class AkilzonShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonSpreadForStaticDisruptionTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonSpreadForStaticDisruptionTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon spread for static disruption") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonElectricalStormIncomingTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonElectricalStormIncomingTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon electrical storm incoming") {}

protected:
    bool IsActiveInEncounter() override;
};

class AkilzonShouldTrackElectricalStormTrigger : public ZulAmanEncounterTrigger
{
public:
    AkilzonShouldTrackElectricalStormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "akil'zon should track electrical storm") {}

protected:
    bool IsActiveInEncounter() override;
};

// Nalorakk <Bear Avatar>

class NalorakkBothFormsShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    NalorakkBothFormsShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "nalorakk both forms should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class NalorakkSpreadForSurgeTrigger : public ZulAmanEncounterTrigger
{
public:
    NalorakkSpreadForSurgeTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "nalorakk spread for surge") {}

protected:
    bool IsActiveInEncounter() override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiSpreadForFlameBreathTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiSpreadForFlameBreathTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai spread for flame breath") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiIsFireBombingTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiIsFireBombingTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai is fire bombing") {}

protected:
    bool IsActiveInEncounter() override;
};

class JanalaiAmanishiHatchersSpawnedTrigger : public ZulAmanEncounterTrigger
{
public:
    JanalaiAmanishiHatchersSpawnedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "jan'alai amani'shi hatchers spawned") {}

protected:
    bool IsActiveInEncounter() override;
};

// Halazzi <Lynx Avatar>

class HalazziShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class HalazziSpiritLynxHasAppearedTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziSpiritLynxHasAppearedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi spirit lynx has appeared") {}

protected:
    bool IsActiveInEncounter() override;
};

class HalazziShouldFocusDpsTrigger : public ZulAmanEncounterTrigger
{
public:
    HalazziShouldFocusDpsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "halazzi should focus dps") {}

protected:
    bool IsActiveInEncounter() override;
};

// Hex Lord Malacrass

class HexLordMalacrassShouldPrioritizeAddsTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassShouldPrioritizeAddsTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass should prioritize adds") {}

protected:
    bool IsActiveInEncounter() override;
};

class HexLordMalacrassChannelingWhirlwindTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassChannelingWhirlwindTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass channeling whirlwind") {}

protected:
    bool IsActiveInEncounter() override;
};

class HexLordMalacrassFreezingTrapPlacedTrigger : public ZulAmanEncounterTrigger
{
public:
    HexLordMalacrassFreezingTrapPlacedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "hex lord malacrass freezing trap placed") {}

protected:
    bool IsActiveInEncounter() override;
};

// Zul'jin

class ZuljinShouldBeTankedTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinShouldBeTankedTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin should be tanked") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinChannelingWhirlwindInTrollFormTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinChannelingWhirlwindInTrollFormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin channeling whirlwind in troll form") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinCreepingParalysisInBearFormTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinCreepingParalysisInBearFormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin creeping paralysis in bear form") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinSummoningCyclonesInEagleFormTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinSummoningCyclonesInEagleFormTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin summoning cyclones in eagle form") {}

protected:
    bool IsActiveInEncounter() override;
};

class ZuljinSpreadForDragonhawkAoeTrigger : public ZulAmanEncounterTrigger
{
public:
    ZuljinSpreadForDragonhawkAoeTrigger(PlayerbotAI* botAI)
        : ZulAmanEncounterTrigger(botAI, "zul'jin spread for dragonhawk aoe") {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
