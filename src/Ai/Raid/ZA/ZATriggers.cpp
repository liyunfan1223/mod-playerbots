/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZATriggers.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"
#include "ZAHelpers.h"

using namespace ZaHelpers;
using namespace EncounterHelpers;

// General

bool ZulAmanNoEncounterInProgressTrigger::IsActive()
{
    if (IsEncounterInProgress(bot, ZA_MAP_ID))
        return false;

    return IsMechanicTrackerBot(bot, ZA_MAP_ID);
}

bool ZulAmanPullingBossTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", _bossName);
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool AmanishiMedicineManSummonedWardTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) &&
        AI_VALUE2(Unit*, "find target", "amani'shi medicine man");
}

// Akil'zon <Eagle Avatar>

bool AkilzonShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    return !GetElectricalStormTarget(bot);
}

bool AkilzonSpreadForStaticDisruptionTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return true;

    return !IsInStormWindow(it->second);
}

bool AkilzonElectricalStormIncomingTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "akil'zon"))
        return false;

    auto it = akilzonStormTimer.find(bot->GetInstanceId());
    if (it == akilzonStormTimer.end())
        return false;

    return IsInStormWindow(it->second);
}

bool AkilzonShouldTrackElectricalStormTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, ZA_MAP_ID) && AI_VALUE2(Unit*, "find target", "akil'zon");
}

// Nalorakk <Bear Avatar>

bool NalorakkBothFormsShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "nalorakk"))
        return false;

    return PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool NalorakkSpreadForSurgeTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "nalorakk");
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    return janalai && !IsJanalaiBombing(janalai);
}

bool JanalaiSpreadForFlameBreathTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (AI_VALUE2(Unit*, "find target", "amani dragonhawk hatchling"))
        return false;

    return !IsJanalaiBombing(janalai);
}

bool JanalaiIsFireBombingTrigger::IsActiveInEncounter()
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    return IsJanalaiBombing(janalai);
}

bool JanalaiAmanishiHatchersSpawnedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai || janalai->GetHealthPct() <= JANALAI_HATCH_ALL_HEALTH_PCT)
        return false;

    return bot->FindNearestCreature(Id(ZaNpcs::NPC_AMANISHI_HATCHER), ZA_CREATURE_SEARCH_RADIUS);
}

// Halazzi <Lynx Avatar>

bool HalazziShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziSpiritLynxHasAppearedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "halazzi");
}

bool HalazziShouldFocusDpsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "halazzi");
}

// Hex Lord Malacrass

bool HexLordMalacrassShouldPrioritizeAddsTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsDps(bot) && AI_VALUE2(Unit*, "find target", "hex lord malacrass");
}

bool HexLordMalacrassChannelingWhirlwindTrigger::IsActiveInEncounter()
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass || malacrass->GetVictim() == bot)
        return false;

    return malacrass->HasAura(Id(ZaSpells::SPELL_HEX_LORD_WHIRLWIND));
}

bool HexLordMalacrassFreezingTrapPlacedTrigger::IsActiveInEncounter()
{
    if (!AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
        return false;

    return GetNearbyFreezingTrap(botAI);
}

// Zul'jin

bool ZuljinShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    // Eagle can't be tanked, and flexibility for the tank to turn Zul'jin during Dragonhawk
    // is preferable.
    return !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)) &&
        !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}

bool ZuljinChannelingWhirlwindInTrollFormTrigger::IsActiveInEncounter()
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_ZULJIN_WHIRLWIND)))
        return false;

    return !PlayerbotAI::IsTank(bot) || zuljin->GetVictim() != bot;
}

bool ZuljinCreepingParalysisInBearFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin || !zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_BEAR)))
        return false;

    return GetZuljinCreepingParalysisDispelTarget(bot);
}

bool ZuljinSummoningCyclonesInEagleFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    if (zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_EAGLE)))
        return true;

    // The aura check is cleaner, but the health check here allows ranged to head to their
    // positions during the phase transition sequence.
    float const healthPct = zuljin->GetHealthPct();
    return healthPct <= 60.0f && healthPct > 40.0f;
}

bool ZuljinSpreadForDragonhawkAoeTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    return zuljin && zuljin->HasAura(Id(ZaSpells::SPELL_SHAPE_OF_THE_DRAGONHAWK));
}
