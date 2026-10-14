/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZAStrategy.h"
#include "ZAMultipliers.h"

void RaidZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("zul'aman no encounter in progress", {
        NextAction("zul'aman reset encounter states", ACTION_EMERGENCY + 10) }));

    // Trash
    triggers.push_back(new TriggerNode("amani'shi medicine man summoned ward", {
        NextAction("amani'shi medicine man mark ward", ACTION_RAID) }));

    // Akil'zon <Eagle Avatar>
    triggers.push_back(new TriggerNode("akil'zon pulling boss", {
        NextAction("akil'zon misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("akil'zon should be tanked", {
        NextAction("akil'zon tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("akil'zon spread for static disruption", {
        NextAction("akil'zon spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("akil'zon electrical storm incoming", {
        NextAction("akil'zon move to eye of the storm", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("akil'zon should track electrical storm", {
        NextAction("akil'zon start electrical storm timer", ACTION_EMERGENCY + 10) }));

    // Nalorakk <Bear Avatar>
    triggers.push_back(new TriggerNode("nalorakk pulling boss", {
        NextAction("nalorakk misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nalorakk both forms should be tanked", {
        NextAction("nalorakk tanks position boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nalorakk spread for surge", {
        NextAction("nalorakk spread ranged", ACTION_RAID) }));

    // Jan'alai <Dragonhawk Avatar>
    triggers.push_back(new TriggerNode("jan'alai pulling boss", {
        NextAction("jan'alai misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("jan'alai should be tanked", {
        NextAction("jan'alai tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("jan'alai spread for flame breath", {
        NextAction("jan'alai spread ranged in circle", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("jan'alai is fire bombing", {
        NextAction("jan'alai avoid fire bombs", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("jan'alai amani'shi hatchers spawned", {
        NextAction("jan'alai mark amani'shi hatchers", ACTION_RAID + 1) }));

    // Halazzi <Lynx Avatar>
    triggers.push_back(new TriggerNode("halazzi pulling boss", {
        NextAction("halazzi misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("halazzi should be tanked", {
        NextAction("halazzi main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("halazzi spirit lynx has appeared", {
        NextAction("halazzi first assist tank attack spirit lynx", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("halazzi should focus dps", {
        NextAction("halazzi dps attack totem and boss", ACTION_RAID) }));

    // Hex Lord Malacrass
    triggers.push_back(new TriggerNode("hex lord malacrass pulling boss", {
        NextAction("hex lord malacrass misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hex lord malacrass should prioritize adds", {
        NextAction("hex lord malacrass assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("hex lord malacrass channeling whirlwind", {
        NextAction("hex lord malacrass run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("hex lord malacrass freezing trap placed", {
        NextAction("hex lord malacrass move away from freezing trap", ACTION_EMERGENCY + 1) }));

    // Zul'jin
    triggers.push_back(new TriggerNode("zul'jin pulling boss", {
        NextAction("zul'jin misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("zul'jin should be tanked", {
        NextAction("zul'jin tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("zul'jin channeling whirlwind in troll form", {
        NextAction("zul'jin run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("zul'jin creeping paralysis in bear form", {
        NextAction("zul'jin mass dispel creeping paralysis", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("zul'jin summoning cyclones in eagle form", {
        NextAction("zul'jin position ranged for cyclones", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("zul'jin spread for dragonhawk aoe", {
        NextAction("zul'jin spread ranged", ACTION_RAID) }));
}

void RaidZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new ZulAmanDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new ZulAmanDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new ZulAmanControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new ZulAmanDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new ZulAmanAvoidWhirlwindMultiplier(botAI));

    // Akil'zon <Eagle Avatar>
    multipliers.push_back(new AkilzonStayInEyeOfTheStormMultiplier(botAI));

    // Jan'alai <Dragonhawk Avatar>
    multipliers.push_back(new JanalaiStayAwayFromFireBombsMultiplier(botAI));
    multipliers.push_back(new JanalaiDoNotCrowdControlHatchersMultiplier(botAI));

    // Halazzi <Lynx Avatar>
    multipliers.push_back(new HalazziDisableAutoDpsTargetingMultiplier(botAI));

    // Hex Lord Malacrass
    multipliers.push_back(new HexLordMalacrassUnstableAfflictionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassSpellReflectionMultiplier(botAI));
    multipliers.push_back(new HexLordMalacrassStayAwayFromFreezingTrapMultiplier(botAI));

    // Zul'jin
    multipliers.push_back(new ZuljinStopAttackingDuringPhaseChangeMultiplier(botAI));
    multipliers.push_back(new ZuljinEagleDisableAvoidAoeMultiplier(botAI));
}
