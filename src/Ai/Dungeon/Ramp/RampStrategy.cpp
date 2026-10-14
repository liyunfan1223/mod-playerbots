/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RampStrategy.h"
#include "RampMultipliers.h"

void TbcDungeonHellfireRampartsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Watchkeeper Gargolmar
    triggers.push_back(new TriggerNode("gargolmar hellfire watchers are active",
                                       {NextAction("gargolmar mark hellfire watchers", ACTION_RAID)}));

    // Omor the Unscarred
    triggers.push_back(new TriggerNode("omor treachery aura",
                                       {NextAction("omor treachery aura flee from players", ACTION_EMERGENCY + 1)}));

    triggers.push_back(new TriggerNode("omor tank has treachery aura",
                                       {NextAction("omor treachery aura flee from tank", ACTION_EMERGENCY + 1)}));

    triggers.push_back(new TriggerNode("omor ranged spread",
                                        {NextAction("omor ranged spread", ACTION_RAID + 1)}));

    triggers.push_back(new TriggerNode("omor fiendish hound is active",
                                        {NextAction("omor mark fiendish hound", ACTION_RAID)}));

    // Vazruden & Nazan
    triggers.push_back(new TriggerNode("vazruden tank position boss",
                                        {NextAction("vazruden tank position boss", ACTION_RAID)}));

    triggers.push_back(new TriggerNode("vazruden boss is active",
                                        {NextAction("vazruden mark boss", ACTION_RAID + 1)}));

    triggers.push_back(new TriggerNode("nazan boss tremor totem",
                                        {NextAction("nazan set tremor totem", ACTION_RAID + 1)}));

    triggers.push_back(new TriggerNode("nazan boss fire resistance totem",
                                        {NextAction("nazan set fire resistance totem", ACTION_RAID)}));

    triggers.push_back(new TriggerNode("nazan boss fire resistance aura",
                                       {NextAction("nazan set fire resistance aura", ACTION_RAID)}));
}

void TbcDungeonHellfireRampartsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new OmorTreacheryAuraFleeFromPlayersMultiplier(botAI));

    multipliers.push_back(new NazanSetTremorTotemMultiplier(botAI));

    multipliers.push_back(new NazanSetFireResistanceTotemMultiplier(botAI));

    multipliers.push_back(new NazanSetFireResistanceAuraMultiplier(botAI));
}
