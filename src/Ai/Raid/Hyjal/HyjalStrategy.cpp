/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HyjalStrategy.h"
#include "HyjalMultipliers.h"

void RaidHyjalStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("hyjal no encounter in progress", {
        NextAction("hyjal reset encounter states", ACTION_EMERGENCY + 10) }));

    // Rage Winterchill
    triggers.push_back(new TriggerNode("rage winterchill pulling boss", {
        NextAction("rage winterchill misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill should be tanked", {
        NextAction("rage winterchill main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("rage winterchill ranged should spread", {
        NextAction("rage winterchill spread ranged in circle", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("rage winterchill melee near death and decay", {
        NextAction(
            "rage winterchill melee maneuver through death and decay", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("rage winterchill ranged in death and decay", {
        NextAction("rage winterchill ranged get out of death and decay", ACTION_EMERGENCY + 1) }));

    // Anetheron
    triggers.push_back(new TriggerNode("anetheron pulling boss or infernal", {
        NextAction("anetheron misdirect boss and infernals to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("anetheron should be tanked", {
        NextAction("anetheron main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("anetheron ranged should spread", {
        NextAction("anetheron spread ranged in circle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("anetheron targeted by infernal", {
        NextAction("anetheron bring infernal to infernal tank", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("anetheron near inferno target", {
        NextAction("anetheron move away from inferno target", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("anetheron infernals pulse immolation", {
        NextAction("anetheron get out of immolation", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("anetheron infernals should be tanked away", {
        NextAction("anetheron infernal tank take position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("anetheron should divide dps", {
        NextAction("anetheron assign dps priority", ACTION_RAID) }));

    // Kaz'rogal
    triggers.push_back(new TriggerNode("kaz'rogal pulling boss", {
        NextAction("kaz'rogal misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal should be tanked", {
        NextAction("kaz'rogal main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal can split malevolent cleave damage", {
        NextAction("kaz'rogal assist tanks move in front", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal ranged should avoid war stomp", {
        NextAction("kaz'rogal spread ranged in arc", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kaz'rogal low on mana", {
        NextAction("kaz'rogal move away from group", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("kaz'rogal hunter should preserve mana", {
        NextAction("kaz'rogal activate aspect of the viper", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kaz'rogal mark on mage or paladin", {
        NextAction("hyjal remove dangerous dot", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kaz'rogal immunity no longer needed", {
        NextAction("kaz'rogal cancel immunity", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kaz'rogal warlock should manage mana", {
        NextAction("kaz'rogal warlock manage mana", ACTION_EMERGENCY + 6) }));

    // Azgalor
    triggers.push_back(new TriggerNode("azgalor pulling boss", {
        NextAction("azgalor misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("azgalor should be tanked", {
        NextAction("azgalor main tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("azgalor ranged should spread", {
        NextAction("azgalor disperse ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("azgalor melee near rain of fire", {
        NextAction("azgalor melee maneuver through fire", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("azgalor ranged in rain of fire", {
        NextAction("azgalor ranged get out of rain of fire", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("azgalor bot is doomed", {
        NextAction("azgalor move to doomguard tank", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("azgalor should control doomguards", {
        NextAction("azgalor tank position doomguard", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("azgalor should divide dps", {
        NextAction("azgalor determine dps priority", ACTION_RAID) }));

    // Archimonde
    triggers.push_back(new TriggerNode("archimonde pulling boss", {
        NextAction("archimonde misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("archimonde should be tanked", {
        NextAction("archimonde move boss to initial position", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("archimonde shaman should protect against fear", {
        NextAction("archimonde set tremor totem", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("archimonde casting air burst", {
        NextAction("archimonde keep air burst away from tank", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("archimonde ranged should spread", {
        NextAction("archimonde spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("archimonde near doomfire", {
        NextAction("archimonde avoid doomfire", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("archimonde stood in doomfire", {
        NextAction("hyjal remove dangerous dot", ACTION_EMERGENCY + 7) }));
}

void RaidHyjalStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new HyjalDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new HyjalDisableDisperseAndTankFaceMultiplier(botAI));

    // Rage Winterchill
    multipliers.push_back(new RageWinterchillMeleeControlAvoidanceMultiplier(botAI));
    multipliers.push_back(new RageWinterchillRangedControlAvoidanceMultiplier(botAI));

    // Anetheron
    multipliers.push_back(new AnetheronDisableAssistTargetingMultiplier(botAI));
    multipliers.push_back(new AnetheronAvoidAccidentalInfernalAggroMultiplier(botAI));
    multipliers.push_back(new AnetheronInfernalTargetRunToPositionMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMovementMultiplier(botAI));
    multipliers.push_back(new AnetheronControlMisdirectionMultiplier(botAI));

    // Kaz'rogal
    multipliers.push_back(new KazrogalControlLowManaMovementMultiplier(botAI));
    multipliers.push_back(new KazrogalKeepAspectOfTheViperActiveMultiplier(botAI));

    // Azgalor
    multipliers.push_back(new AzgalorDisableAutoTargetingAndPositioningMultiplier(botAI));
    multipliers.push_back(new AzgalorDoomedBotPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new AzgalorMeleeDpsControlAvoidanceMultiplier(botAI));
    multipliers.push_back(new AzgalorRangedControlAvoidanceMultiplier(botAI));

    // Archimonde
    multipliers.push_back(new ArchimondeControlDoomfireAvoidanceMultiplier(botAI));
    multipliers.push_back(new ArchimondeSetTremorTotemMultiplier(botAI));
}
