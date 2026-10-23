/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SSCStrategy.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SSCHelpers.h"
#include "SSCMultipliers.h"

using namespace SscHelpers;
using namespace EncounterHelpers;

void RaidSscStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shared
    triggers.push_back(new TriggerNode("ssc no encounter in progress",
        { NextAction("ssc reset encounter states", ACTION_EMERGENCY + 10) }));

    // Trash
    triggers.push_back(new TriggerNode("underbog colossus in toxic pool",
        { NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("greyheart tidecaller water elemental totem spawned",
        { NextAction("greyheart tidecaller mark water elemental totem", ACTION_RAID) }));

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode("hydross the unstable should be tanked by frost tank",
        { NextAction("hydross the unstable position frost tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hydross the unstable should be tanked by nature tank",
        { NextAction("hydross the unstable position nature tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hydross the unstable ranged should spread in frost phase",
        { NextAction("hydross the unstable frost phase spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable should misdirect upon phase change",
        { NextAction("hydross the unstable misdirect to tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("hydross the unstable aggro resets upon phase change",
        { NextAction("hydross the unstable stop attacking upon phase change", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("hydross the unstable off phase tank attacking",
        { NextAction("hydross the unstable stop attacking upon phase change", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("hydross the unstable should manage phase timers",
        { NextAction("hydross the unstable manage phase timers", ACTION_EMERGENCY + 10) }));

    // The Lurker Below
    triggers.push_back(new TriggerNode("the lurker below spout is active",
        { NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("the lurker below should be tanked",
        { NextAction("the lurker below position main tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the lurker below ranged should spread",
        { NextAction("the lurker below spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the lurker below guardians should be tanked",
        { NextAction("the lurker below tanks pick up guardians", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the lurker below melee cannot reach target",
        { NextAction("the lurker below melee move directly to target", ACTION_HIGH) }));

    triggers.push_back(new TriggerNode("the lurker below melee in water",
        { NextAction("the lurker below melee get out of water", ACTION_EMERGENCY + 5) }));

    // Leotheras the Blind
    triggers.push_back(new TriggerNode("leotheras the blind ranged should spread upon pull",
        { NextAction("leotheras the blind spread ranged upon pull", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind warlock should tank demon form",
        { NextAction("leotheras the blind warlock tank attack demon form", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind tanks should auto-attack demon form",
        { NextAction("leotheras the blind tanks build rage on demon form", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind ranged should keep distance",
        { NextAction("leotheras the blind ranged keep distance", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("leotheras the blind channeling whirlwind",
        { NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind too many chaos blast stacks",
        { NextAction("leotheras the blind melee run from chaos blast", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("leotheras the blind inner demon has awakened",
        { NextAction("leotheras the blind destroy inner demon", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("leotheras the blind in final phase",
        { NextAction("leotheras the blind final phase attack boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind should separate boss from demon",
        { NextAction(
            "leotheras the blind final phase separate boss from demon", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("leotheras the blind hunter should misdirect demon form",
        { NextAction("leotheras the blind misdirect demon form to tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("leotheras the blind aggro resets",
        { NextAction("leotheras the blind melee stop attacking", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("leotheras the blind should manage dps wait timers",
        { NextAction("leotheras the blind manage dps wait timers", ACTION_EMERGENCY + 10) }));

    // Fathom-Lord Karathress
    triggers.push_back(new TriggerNode("fathom-lord karathress targets should be tanked",
        { NextAction("fathom-lord karathress tanks position targets", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress should heal caribdis tank",
        { NextAction("fathom-lord karathress position caribdis tank healer", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress hunter should misdirect",
        { NextAction("fathom-lord karathress misdirect to tanks", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress should assign dps priority",
        { NextAction("fathom-lord karathress assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress should manage dps timer",
        { NextAction("fathom-lord karathress manage dps timer", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress ranged should spread",
        { NextAction("fathom-lord karathress spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress stuck midair after cyclone",
        { NextAction(
            "fathom-lord karathress drop to ground after cyclone", ACTION_EMERGENCY + 9) }));

    // Morogrim Tidewalker
    triggers.push_back(new TriggerNode("morogrim tidewalker should be tanked",
        { NextAction("morogrim tidewalker position main tank", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker ranged should stack",
        { NextAction("morogrim tidewalker stack ranged behind boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker too far from boss",
        { NextAction("morogrim tidewalker return to boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker hunter should misdirect",
        { NextAction("morogrim tidewalker misdirect to main tank", ACTION_RAID) }));

    // Lady Vashj <Coilfang Matron>
    triggers.push_back(new TriggerNode("lady vashj should be tanked",
        { NextAction("lady vashj main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj ranged should spread in phase 1",
        { NextAction("lady vashj phase 1 spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj station slots need holders",
        { NextAction("lady vashj assign station slots", ACTION_EMERGENCY + 14) }));

    triggers.push_back(new TriggerNode("lady vashj should hold station in phase 2",
        { NextAction("lady vashj phase 2 position at station", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj ranged should position in phase 3",
        { NextAction("lady vashj phase 3 position ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj out of sight in phase 3",
        { NextAction("lady vashj phase 3 move into sight", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj main tank needs grounding shaman",
        { NextAction("lady vashj assign grounding shaman", ACTION_EMERGENCY + 14) }));

    triggers.push_back(new TriggerNode("lady vashj shaman should ground shock blast",
        { NextAction("lady vashj set grounding totem in main tank group", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj static charge on group member",
        { NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("lady vashj hunter should misdirect",
        { NextAction("lady vashj misdirect to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj should assign target priority",
        { NextAction("lady vashj assign target priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("lady vashj tank needs fear ward",
        { NextAction("lady vashj tank apply fear ward", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("lady vashj coilfang strider should be tanked",
        { NextAction("lady vashj position coilfang strider", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj coilfang elite should be tanked",
        { NextAction("lady vashj position coilfang elite", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj tank is idle away from the middle",
        { NextAction("lady vashj tank wait in the middle", ACTION_RAID - 1) }));

    triggers.push_back(new TriggerNode("lady vashj tainted elemental needs looter",
        { NextAction("lady vashj assign tainted core looter", ACTION_EMERGENCY + 13) }));

    triggers.push_back(new TriggerNode("lady vashj should attack tainted elemental",
        { NextAction("lady vashj attack tainted elemental", ACTION_EMERGENCY + 12) }));

    triggers.push_back(new TriggerNode("lady vashj tainted core looter",
        { NextAction("lady vashj loot tainted core", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("lady vashj core passing chain member",
        { NextAction("lady vashj pass the tainted core", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("lady vashj should destroy tainted core",
        { NextAction("lady vashj destroy tainted core", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("lady vashj pet should switch target",
        { NextAction("lady vashj command pet target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("lady vashj bot above the ground",
        { NextAction("lady vashj return to the ground", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("lady vashj bot in toxic spores",
        { NextAction("lady vashj avoid toxic spores", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("lady vashj melee near toxic spores",
        { NextAction("lady vashj melee move around toxic spores", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("lady vashj ranged reach blocked by toxic spores",
        { NextAction("lady vashj ranged reach around toxic spores", ACTION_RAID - 1) }));

    triggers.push_back(new TriggerNode("lady vashj entangle on melee",
        { NextAction("lady vashj paladin use hand of freedom", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("lady vashj static charge on rogue",
        { NextAction("lady vashj rogue use cloak of shadows", ACTION_EMERGENCY + 8) }));
}

void RaidSscStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Shared
    multipliers.push_back(new SscControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new SscDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new SscNoFishingDuringEncounterMultiplier(botAI));

    // Trash
    multipliers.push_back(new UnderbogColossusHoldNearToxicPoolMultiplier(botAI));

    // Hydross the Unstable <Duke of Currents>
    multipliers.push_back(new HydrossTheUnstableDisableOffPhaseTankActionsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableDisablePhaseTankAssistMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDpsMultiplier(botAI));

    // The Lurker Below
    multipliers.push_back(new TheLurkerBelowStayAwayFromSpoutMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMaintainPositionsMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowTanksFocusAssignedGuardianMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowDisableKillingSpreeMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMeleeWaitToSetBehindMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMeleeDisableReachMultiplier(botAI));

    // Leotheras the Blind
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindMeleeAvoidChaosBlastMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindFocusOnInnerDemonMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDpsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankSoulshatterMultiplier(botAI));

    // Fathom-Lord Karathress
    multipliers.push_back(new FathomLordKarathressDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAutoTargetMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAoeMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDpsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressMaintainPositionMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressNoCastingWhileLiftedMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressApproachingCaribdisMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDontDropOutOfSightTargetMultiplier(botAI));

    // Morogrim Tidewalker
    multipliers.push_back(new MorogrimTidewalkerControlMovementMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerStayStackedMultiplier(botAI));

    // Lady Vashj <Coilfang Matron>
    multipliers.push_back(new LadyVashjSetGroundingTotemMultiplier(botAI));
    multipliers.push_back(new LadyVashjMaintainPhase1RangedSpreadMultiplier(botAI));
    multipliers.push_back(new LadyVashjStaticChargeStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new LadyVashjNoUnauthorizedLootingMultiplier(botAI));
    multipliers.push_back(new LadyVashjCoreHandlersPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new LadyVashjPhase2DisableAutoTargetAndMoveMultiplier(botAI));
    multipliers.push_back(new LadyVashjPhase3DisableAutoTargetAndMoveMultiplier(botAI));
    multipliers.push_back(new LadyVashjSaveHandOfFreedomMultiplier(botAI));
    multipliers.push_back(new LadyVashjMeleeControlSporeAvoidanceMultiplier(botAI));
    multipliers.push_back(new LadyVashjRangedDoNotReachThroughSporesMultiplier(botAI));
}

namespace
{

void AppendFathomLordKarathressBlessingHoldExclusions(
    Player* bot, AiObjectContext* context, GuidSet& exclusions)
{
    if (PlayerbotAI::IsHeal(bot) || !PlayerbotAI::IsMelee(bot))
        return;

    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress || karathress->GetHealthPct() > KARATHRESS_BLESSING_HOLD_HEALTH_PCT)
        return;

    if (!AI_VALUE2(Unit*, "find target", "fathom-guard caribdis"))
        return;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return;

    exclusions.insert(karathress->GetGUID());
}

void AppendMorogrimTidewalkerMurlocExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Unit* tidewalker = nullptr;
    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        if (guid.GetEntry() != Id(SscNpcs::NPC_TIDEWALKER_LURKER))
            continue;

        if (!tidewalker)
        {
            tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
            if (!tidewalker)
                return;
        }

        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetExactDist2d(tidewalker) > TIDEWALKER_MURLOC_MAX_TARGET_DISTANCE)
            exclusions.insert(guid);
    }
}

void AppendLadyVashjGeneratorPhaseExclusions(AiObjectContext* context, GuidSet& exclusions)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (vashj && vashj->HasAura(Id(SscSpells::SPELL_MAGIC_BARRIER)))
        exclusions.insert(vashj->GetGUID());
}

} // end anonymous namespace

void RaidSscStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    Player* bot = botAI->GetBot();
    if (!IsEncounterInProgress(bot, SSC_MAP_ID))
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    AppendFathomLordKarathressBlessingHoldExclusions(bot, context, exclusions);
    AppendMorogrimTidewalkerMurlocExclusions(botAI, context, exclusions);
    AppendLadyVashjGeneratorPhaseExclusions(context, exclusions);
}
