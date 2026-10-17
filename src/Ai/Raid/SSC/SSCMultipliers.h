/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCMULTIPLIERS_H
#define PLAYERBOTS_SSCMULTIPLIERS_H

#include "EncounterHelpers.h"
#include "Multiplier.h"
#include "SSCHelpers.h"
#include <string>

// Shared

class SscEncounterMultiplier : public Multiplier
{
public:
    SscEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return EncounterHelpers::IsEncounterInProgress(bot, SscHelpers::SSC_MAP_ID) ?
            GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

// For Lady Vashj, Fathom-Lord Karathress, Hydross, and Leotheras (to Warlock tank).
class SscControlMisdirectionMultiplier : public SscEncounterMultiplier
{
public:
    SscControlMisdirectionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "ssc control misdirection") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Not encounter gated: Leotheras is set to engaged only once something hostile hits him, which may
// not happen until the Spellbinders are dead.
class SscDelayDpsCooldownsMultiplier : public Multiplier
{
public:
    SscDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "ssc delay dps cooldowns") {}

    float GetValue(Action* action) override;
};

class SscNoFishingDuringEncounterMultiplier : public SscEncounterMultiplier
{
public:
    SscNoFishingDuringEncounterMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "ssc no fishing during encounter") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class UnderbogColossusHoldNearToxicPoolMultiplier : public Multiplier
{
public:
    UnderbogColossusHoldNearToxicPoolMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "underbog colossus hold near toxic pool") {}
    float GetValue(Action* action) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableOffPhaseTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableDisableOffPhaseTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable disable off-phase tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HydrossTheUnstableDisablePhaseTankAssistMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableDisablePhaseTankAssistMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable disable phase tank assist") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HydrossTheUnstableWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    HydrossTheUnstableWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "hydross the unstable wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below stay away from spout") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Formation, flee, reposition and follow moves can walk a bot into the water.
class TheLurkerBelowMaintainPositionsMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowMaintainPositionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below maintain positions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheLurkerBelowTanksFocusAssignedGuardianMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowTanksFocusAssignedGuardianMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below tanks focus assigned guardian") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheLurkerBelowDisableKillingSpreeMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowDisableKillingSpreeMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below disable killing spree") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Until his tank has him on the spot he may still be turning, and behind him can be in the
// water. On an islet melee don't move behind anything.
class TheLurkerBelowMeleeWaitToSetBehindMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowMeleeWaitToSetBehindMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below melee wait to set behind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Reach melee's path walked melee off an islet into the water; the direct move does it instead.
class TheLurkerBelowMeleeDisableReachMultiplier : public SscEncounterMultiplier
{
public:
    TheLurkerBelowMeleeDisableReachMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "the lurker below melee disable reach") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind avoid whirlwind") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindMeleeAvoidChaosBlastMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindMeleeAvoidChaosBlastMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind melee avoid chaos blast") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindFocusOnInnerDemonMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind focus on inner demon") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LeotherasTheBlindDisableTankSoulshatterMultiplier : public SscEncounterMultiplier
{
public:
    LeotherasTheBlindDisableTankSoulshatterMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "leotheras the blind disable tank soulshatter") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableTankActionsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable tank actions") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressDisableAutoTargetMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableAutoTargetMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable auto target") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressDisableAoeMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress disable aoe") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressWaitForDpsMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress wait for dps") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressMaintainPositionMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressMaintainPositionMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress maintain position") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressNoCastingWhileLiftedMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressNoCastingWhileLiftedMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress no casting while lifted") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressApproachingCaribdisMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressApproachingCaribdisMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress approaching caribdis") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class FathomLordKarathressDontDropOutOfSightTargetMultiplier : public SscEncounterMultiplier
{
public:
    FathomLordKarathressDontDropOutOfSightTargetMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "fathom-lord karathress don't drop out of sight target") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerControlMovementMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerControlMovementMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker control movement") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MorogrimTidewalkerStayStackedMultiplier : public SscEncounterMultiplier
{
public:
    MorogrimTidewalkerStayStackedMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "morogrim tidewalker stay stacked") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjSetGroundingTotemMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjSetGroundingTotemMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj set grounding totem") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjMaintainPhase1RangedSpreadMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj maintain phase 1 ranged spread") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj static charge stay away from group") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjNoUnauthorizedLootingMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjNoUnauthorizedLootingMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj no unauthorized looting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjCoreHandlersPrioritizePositioningMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjCoreHandlersPrioritizePositioningMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj core handlers prioritize positioning") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjPhase2DisableAutoTargetAndMoveMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjPhase2DisableAutoTargetAndMoveMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj phase 2 disable auto target and move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjPhase3DisableAutoTargetAndMoveMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjPhase3DisableAutoTargetAndMoveMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj phase 3 disable auto target and move") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjSaveHandOfFreedomMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjSaveHandOfFreedomMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj save hand of freedom") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjMeleeControlSporeAvoidanceMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjMeleeControlSporeAvoidanceMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj melee control spore avoidance") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class LadyVashjRangedDoNotReachThroughSporesMultiplier : public SscEncounterMultiplier
{
public:
    LadyVashjRangedDoNotReachThroughSporesMultiplier(PlayerbotAI* botAI)
        : SscEncounterMultiplier(botAI, "lady vashj ranged do not reach through spores") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
