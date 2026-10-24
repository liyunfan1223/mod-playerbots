/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPVALUECONTEXT_H
#define PLAYERBOTS_SWPVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "Position.h"
#include "Value.h"
#include <vector>

class EredarTwinsBlazePositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    EredarTwinsBlazePositionsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "eredar twins blaze", SwpHelpers::EREDAR_TWINS_BLAZE_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return SwpHelpers::FindEredarTwinsBlazePositions(bot);
    }
};

class MuruEncounterTargetsValue : public CalculatedValue<SwpHelpers::MuruEncounterGuids>
{
public:
    MuruEncounterTargetsValue(PlayerbotAI* botAI)
        : CalculatedValue<SwpHelpers::MuruEncounterGuids>(
              botAI, "muru encounter targets",
              SwpHelpers::MURU_ENCOUNTER_TARGETS_CACHE_INTERVAL_MS) {}

protected:
    SwpHelpers::MuruEncounterGuids Calculate() override
    {
        return SwpHelpers::FindMuruEncounterGuids(botAI);
    }
};

class MuruVoidZonesValue : public CalculatedValue<GuidVector>
{
public:
    MuruVoidZonesValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "muru void zones", SwpHelpers::VOID_ZONE_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindMuruVoidZoneGuids(bot); }
};

class FelmystDemonicVaporsValue : public CalculatedValue<SwpHelpers::FelmystDemonicVaporGuids>
{
public:
    FelmystDemonicVaporsValue(PlayerbotAI* botAI)
        : CalculatedValue<SwpHelpers::FelmystDemonicVaporGuids>(
              botAI, "felmyst demonic vapors", SwpHelpers::DEMONIC_VAPOR_CACHE_INTERVAL_MS) {}

protected:
    SwpHelpers::FelmystDemonicVaporGuids Calculate() override
    {
        return SwpHelpers::FindFelmystDemonicVaporGuids(bot);
    }
};

class MuruDarkFiendsValue : public CalculatedValue<GuidVector>
{
public:
    MuruDarkFiendsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "muru dark fiends", SwpHelpers::DARK_FIEND_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindMuruDarkFiendGuids(bot); }
};

class SwpVolatileFiendValue : public CalculatedValue<ObjectGuid>
{
public:
    SwpVolatileFiendValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "swp volatile fiend", SwpHelpers::VOLATILE_FIEND_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindSwpVolatileFiendGuid(bot); }
};

class KalecgosSpectralRiftValue : public CalculatedValue<ObjectGuid>
{
public:
    KalecgosSpectralRiftValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "kalecgos spectral rift", SwpHelpers::SPECTRAL_RIFT_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindKalecgosSpectralRiftGuid(bot); }
};

class MuruSingularityValue : public CalculatedValue<ObjectGuid>
{
public:
    MuruSingularityValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(
              botAI, "muru singularity", SwpHelpers::SINGULARITY_CACHE_INTERVAL_MS) {}

protected:
    ObjectGuid Calculate() override { return SwpHelpers::FindMuruSingularityGuid(bot); }
};

class KiljaedenDragonOrbsValue : public CalculatedValue<GuidVector>
{
public:
    KiljaedenDragonOrbsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "kiljaeden dragon orbs", SwpHelpers::DRAGON_ORB_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindKiljaedenDragonOrbGuids(bot); }
};

class KiljaedenHandsValue : public CalculatedValue<GuidVector>
{
public:
    KiljaedenHandsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "kiljaeden hands", SwpHelpers::HAND_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return SwpHelpers::FindKiljaedenHandGuids(bot); }
};

class RaidSwpValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidSwpValueContext()
    {
        creators["felmyst demonic vapors"] = &RaidSwpValueContext::felmyst_demonic_vapors;
        creators["eredar twins blaze"] = &RaidSwpValueContext::eredar_twins_blaze;
        creators["muru encounter targets"] = &RaidSwpValueContext::muru_encounter_targets;
        creators["muru void zones"] = &RaidSwpValueContext::muru_void_zones;
        creators["muru dark fiends"] = &RaidSwpValueContext::muru_dark_fiends;
        creators["swp volatile fiend"] = &RaidSwpValueContext::swp_volatile_fiend;
        creators["kalecgos spectral rift"] = &RaidSwpValueContext::kalecgos_spectral_rift;
        creators["muru singularity"] = &RaidSwpValueContext::muru_singularity;
        creators["kiljaeden dragon orbs"] = &RaidSwpValueContext::kiljaeden_dragon_orbs;
        creators["kiljaeden hands"] = &RaidSwpValueContext::kiljaeden_hands;
    }

private:
    static UntypedValue* felmyst_demonic_vapors(PlayerbotAI* botAI)
    {
        return new FelmystDemonicVaporsValue(botAI);
    }
    static UntypedValue* eredar_twins_blaze(PlayerbotAI* botAI)
    {
        return new EredarTwinsBlazePositionsValue(botAI);
    }
    static UntypedValue* muru_encounter_targets(PlayerbotAI* botAI)
    {
        return new MuruEncounterTargetsValue(botAI);
    }
    static UntypedValue* muru_void_zones(PlayerbotAI* botAI)
    {
        return new MuruVoidZonesValue(botAI);
    }
    static UntypedValue* muru_dark_fiends(PlayerbotAI* botAI)
    {
        return new MuruDarkFiendsValue(botAI);
    }
    static UntypedValue* swp_volatile_fiend(PlayerbotAI* botAI)
    {
        return new SwpVolatileFiendValue(botAI);
    }
    static UntypedValue* kalecgos_spectral_rift(PlayerbotAI* botAI)
    {
        return new KalecgosSpectralRiftValue(botAI);
    }
    static UntypedValue* muru_singularity(PlayerbotAI* botAI)
    {
        return new MuruSingularityValue(botAI);
    }
    static UntypedValue* kiljaeden_dragon_orbs(PlayerbotAI* botAI)
    {
        return new KiljaedenDragonOrbsValue(botAI);
    }
    static UntypedValue* kiljaeden_hands(PlayerbotAI* botAI)
    {
        return new KiljaedenHandsValue(botAI);
    }
};

#endif
