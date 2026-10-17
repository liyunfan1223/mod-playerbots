/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPMULTIPLIERS_H
#define PLAYERBOTS_SWPMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "SWPShared.h"
#include <string>

// Shared

class SunwellEncounterMultiplier : public Multiplier
{
public:
    SunwellEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID) ?
            GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

class SunwellNoEncounterDrinkingMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellNoEncounterDrinkingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell no encounter drinking") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class VolatileFiendRestrictApproachMultiplier : public Multiplier
{
public:
    VolatileFiendRestrictApproachMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "volatile fiend restrict approach") {}
    float GetValue(Action* action) override;
};

// Shared Boss

class SunwellControlMisdirectionMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SunwellDelayDpsCooldownsMultiplier : public SunwellEncounterMultiplier
{
public:
    SunwellDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "sunwell delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;

private:
    uint32 _tempLogMs = 0; // TEMP LOG
};

// Kalecgos

class KalecgosWaitToDecurseMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosWaitToDecurseMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kalecgos wait to decurse") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kalecgos control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosRestrictTauntMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kalecgos restrict taunt") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosSuppressAssistTankPullThreatMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosSuppressAssistTankPullThreatMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kalecgos suppress assist tank pull threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KalecgosEnterSpectralRiftMultiplier : public SunwellEncounterMultiplier
{
public:
    KalecgosEnterSpectralRiftMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kalecgos enter spectral rift") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Brutallus

class BrutallusControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "brutallus control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusNoKillingSpreeWhenNearbyBurnMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "brutallus no killing spree when nearby burn") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BrutallusRestrictTauntMultiplier : public SunwellEncounterMultiplier
{
public:
    BrutallusRestrictTauntMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "brutallus restrict taunt") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Felmyst

class FelmystControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystWaitForLandingDpsMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystWaitForLandingDpsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst wait for landing dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeEncapsulateAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeEncapsulateAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst prioritize encapsulate avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeFogAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeFogAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst prioritize fog avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystPrioritizeDemonicVaporAvoidanceMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystPrioritizeDemonicVaporAvoidanceMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst prioritize demonic vapor avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystFocusAttacksOnCharmedPlayerMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystFocusAttacksOnCharmedPlayerMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst focus attacks on charmed player") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FelmystDontDotAddsMultiplier : public SunwellEncounterMultiplier
{
public:
    FelmystDontDotAddsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "felmyst don't dot adds") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Eredar Twins

class EredarTwinsDisableAutoTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsDisableAutoTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "eredar twins disable auto targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsHoldDpsAtStartMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsHoldDpsAtStartMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "eredar twins hold dps at start") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlThreatMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsControlThreatMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "eredar twins control threat") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "eredar twins control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class EredarTwinsIsolateConflagrationMultiplier : public SunwellEncounterMultiplier
{
public:
    EredarTwinsIsolateConflagrationMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "eredar twins isolate conflagration") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// M'uru

class MuruDisableDefaultTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruDisableDefaultTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "m'uru disable default targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MuruControlMovementMultiplier : public SunwellEncounterMultiplier
{
public:
    MuruControlMovementMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "m'uru control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Kil'jaeden <The Deceiver>

class KiljaedenSingleTargetHandsMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenSingleTargetHandsMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kil'jaeden single target hands") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenControlMovementAndTargetingMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kil'jaeden control movement and targeting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenPrioritizeDarknessProtectionMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenPrioritizeDarknessProtectionMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kil'jaeden prioritize darkness protection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class KiljaedenControlDragonMultiplier : public SunwellEncounterMultiplier
{
public:
    KiljaedenControlDragonMultiplier(PlayerbotAI* botAI)
        : SunwellEncounterMultiplier(botAI, "kil'jaeden dragon buff and protect raid") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
