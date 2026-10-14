/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MgTStrategy.h"
#include "MgTMultipliers.h"
#include "MgTTriggers.h"
#include "Playerbots.h"

void TbcDungeonMagistersTerraceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mgt out of room", {
        NextAction("mgt return to room", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("mgt crystal active", {
        NextAction("mgt kill crystal", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("mgt in dampening field", {
        NextAction("mgt leave dampening field", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("mgt priority interrupt", {
        NextAction("mgt priority interrupt", ACTION_INTERRUPT + 5) }));

    triggers.push_back(new TriggerNode("mgt arcane nova range", {
        NextAction("mgt clear arcane nova", ACTION_EMERGENCY + 4) }));

    triggers.push_back(new TriggerNode("mgt mage guard at range", {
        NextAction("mgt close on mage guard", ACTION_MOVE + 5) }));

    triggers.push_back(new TriggerNode("mgt enraged wretched", {
        NextAction("mgt taunt enraged wretched", ACTION_HIGH + 2) }));

    triggers.push_back(new TriggerNode("mgt focus target", {
        NextAction("mgt focus target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mgt delrissa interrupt", {
        NextAction("mgt delrissa interrupt", ACTION_INTERRUPT + 5) }));

    triggers.push_back(new TriggerNode("mgt delrissa focus target", {
        NextAction("mgt delrissa focus target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mgt delrissa tremor totem", {
        NextAction("mgt delrissa tremor totem", ACTION_HIGH + 5) }));

    triggers.push_back(new TriggerNode("mgt flame strike", {
        NextAction("mgt leave flame strike", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("mgt phoenix burn", {
        NextAction("mgt leave phoenix burn", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("mgt kael focus target", {
        NextAction("mgt kael focus target", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mgt kael interrupt", {
        NextAction("mgt kael interrupt", ACTION_INTERRUPT + 6) }));

    triggers.push_back(new TriggerNode("mgt gravity lapse", {
        NextAction("mgt take lapse spot", ACTION_MOVE + 8) }));
}

void TbcDungeonMagistersTerraceStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MgTCrystalFocusMultiplier(botAI));
    multipliers.push_back(new MgTFocusOrderMultiplier(botAI));
    multipliers.push_back(new MgTFocusBurstMultiplier(botAI));
    multipliers.push_back(new MgTRoomLeashMultiplier(botAI));
    multipliers.push_back(new MgTDampeningFieldMultiplier(botAI));
    multipliers.push_back(new MgTDelrissaTremorTotemMultiplier(botAI));
    multipliers.push_back(new MgTFlameStrikeMultiplier(botAI));
    multipliers.push_back(new MgTPhoenixBurnMultiplier(botAI));
    multipliers.push_back(new MgTKaelUnattackableMultiplier(botAI));
    multipliers.push_back(new MgTGravityLapseMultiplier(botAI));
}

void TbcDungeonMagistersTerraceStrategy::AppendTargetExclusions(GuidSet& exclusions,
                                                               TargetValueExclusionType type)
{
    if (type == TargetValueExclusionType::Aoe)
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();

    for (ObjectGuid const guid : context->GetValue<GuidVector>("mgt delrissa pets")->Get())
        exclusions.insert(guid);

    for (ObjectGuid const guid : context->GetValue<GuidVector>("mgt melee phoenix exclusions")->Get())
        exclusions.insert(guid);

    if (type != TargetValueExclusionType::DebuffTarget)
        return;

    for (ObjectGuid const guid : context->GetValue<GuidVector>("mgt focus exclusions")->Get())
        exclusions.insert(guid);
}
