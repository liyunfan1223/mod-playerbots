/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLStrategy.h"
#include "BWLHelpers.h"
#include "BWLMultipliers.h"
#include "Playerbots.h"

void RaidBwlStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", {
        NextAction("bwl check onyxia scale cloak", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl suppression device", {
        NextAction("bwl turn off suppression device", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl razorgore fire resistance", {
        NextAction("bwl razorgore fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl razorgore not mind controlled", {
        NextAction("bwl razorgore avoid aoe", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl razorgore not mind controlled", {
        NextAction("bwl razorgore mark boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("bwl vaelastrasz fire resistance", {
        NextAction("bwl vaelastrasz fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl vaelastrasz positioning", {
        NextAction("rear flank", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("bwl vaelastrasz burning adrenaline", {
        NextAction("bwl vaelastrasz move away", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("bwl broodlord fire resistance", {
        NextAction("bwl broodlord fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl broodlord ranged too close", {
        NextAction("bwl broodlord ranged move away", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl firemaw fire resistance", {
        NextAction("bwl firemaw fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl firemaw not victim", {
        NextAction("bwl firemaw avoid breath", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("bwl ebonroc not victim", {
        NextAction("bwl ebonroc avoid breath", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("bwl flamegor fire resistance", {
        NextAction("bwl flamegor fire resistance", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl flamegor not victim", {
        NextAction("bwl flamegor avoid breath", ACTION_MOVE + 4) }));

    triggers.push_back(new TriggerNode("bwl affliction bronze", {
        NextAction("bwl use hourglass sand", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl nefarian positioning", {
        NextAction("bwl nefarian rear flank", ACTION_MOVE + 4) }));
    triggers.push_back(new TriggerNode("bwl nefarian wild magic", {
        NextAction("ice block", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("bwl death talon wyrmguard tank", {
        NextAction("bwl death talon wyrmguard tank move away", ACTION_RAID) }));
    triggers.push_back(new TriggerNode("bwl death talon wyrmguard ranged", {
        NextAction("bwl death talon wyrmguard ranged move away", ACTION_RAID) }));
}

void RaidBwlStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RazorgoreTankMultiplier(botAI));
    multipliers.push_back(new VaelastraszTankMultiplier(botAI));
    multipliers.push_back(new VaelastraszBurningAdrenalineMultiplier(botAI));
}

void RaidBwlStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::TankTarget)
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (Unit* broodlord = AI_VALUE2(Unit*, "find target", "broodlord lashlayer"))
    {
        Unit* bossVictim = broodlord->GetVictim();
        Player* bossVictimPlayer = bossVictim ? bossVictim->ToPlayer() : nullptr;

        Unit* bot = botAI->GetBot();
        if (!bossVictimPlayer || !PlayerbotAI::IsTank(bossVictimPlayer) || bossVictim == bot)
        {
            // Only the tank actually holding the boss keeps it as its target at all times.
            // All other tanks may switch to non-whelp targets (other branch).
            // This should not happen, if the elites were killed before the encounter, but just in case...
            // If the boss targets a non-tank, all tanks should ignore the adds and attack the boss only.
            for (ObjectGuid const guid : AI_VALUE(GuidVector, "attackers"))
            {
                Unit* unit = botAI->GetUnit(guid);
                if (unit != broodlord)
                    exclusions.insert(guid);
            }
        }
        else
        {
            // Prevents the tanks from changing targets to a whelp during the encounter.
            // Tanks should build threat on the boss. DPS handles whelps on their own.
            // Change target if it isn't a whelp (boss or respawned elite NPCs).
            for (ObjectGuid const guid : AI_VALUE(GuidVector, "attackers"))
            {
                Unit* unit = botAI->GetUnit(guid);
                if (BlackwingLairHelpers::IsCorruptedWhelp(unit))
                    exclusions.insert(guid);
            }
        }
    }
}
