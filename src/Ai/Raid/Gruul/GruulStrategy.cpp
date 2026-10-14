/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulStrategy.h"
#include "GruulMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("gruul's lair no encounter in progress", {
        NextAction("gruul's lair reset encounter states", ACTION_EMERGENCY + 10) }));

    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar three ogres need melee tanks", {
        NextAction("high king maulgar melee tanks position bosses", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar krosh needs mage tank", {
        NextAction("high king maulgar mage tank attack krosh", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar kiggler needs moonkin tank", {
        NextAction("high king maulgar moonkin tank attack kiggler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar determining kill order", {
        NextAction("high king maulgar assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("high king maulgar channeling whirlwind", {
        NextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("high king maulgar should stand back from krosh", {
        NextAction("high king maulgar back away from krosh", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("high king maulgar wild fel stalker spawned", {
        NextAction("high king maulgar banish fel stalker", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar pulling ogre council", {
        NextAction("high king maulgar misdirect ogres to tanks", ACTION_RAID + 1) }));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller should be tanked", {
        NextAction("gruul the dragonkiller tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller ranged should spread", {
        NextAction("gruul the dragonkiller spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller in cave in", {
        NextAction("gruul the dragonkiller get out of cave in", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller incoming shatter", {
        NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6) }));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // General
    multipliers.push_back(new GruulsLairDelayDpsCooldownsMultiplier(botAI));

    // High King Maulgar
    multipliers.push_back(new HighKingMaulgarControlTankActionsMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarRestrictTauntingMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableDpsAssistMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarControlHunterActionsMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarControlMageTankActionsMultiplier(botAI));

    // Gruul the Dragonkiller
    multipliers.push_back(new GruulTheDragonkillerControlTankMovementMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerStaySpreadForShatterMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerControlAvoidanceMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerHoldWhileSnaredMultiplier(botAI));
}
