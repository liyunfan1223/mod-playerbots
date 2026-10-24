/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldStrategy.h"

#include "Playerbots.h"
#include "UldDefinitions.h"
#include "UldEncounter_Algalon.h"
#include "UldEncounter_Mimiron.h"
#include "UldEncounter_YoggSaron.h"
#include "UldMultipliers.h"

// The kept Emergency Fire Bots put out Mimiron's fire. Excluding them here covers every picker at once,
// tank, dps, dps aoe and attackers, where Mimiron's own guards only reach dps assist
void RaidUlduarStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    for (ObjectGuid const& guid : GetMimironKeptFireBots(botAI, botAI->GetBot()))
        exclusions.insert(guid);

    AppendAlgalonTargetExclusions(botAI, exclusions);

    // Dps and pets only, the tank still has to hold the shielded Guardian or it walks at the back line.
    // Stops the DoTs, cleaves and pets the focus rule misses from killing it beside its neighbour
    if (type == TargetValueExclusionType::DpsTarget || type == TargetValueExclusionType::DebuffTarget)
        AppendYoggSaronTargetExclusions(botAI, exclusions);
}

void RaidUlduarStrategy::OnTick()
{
    for (EncounterDefinition const* encounter : UldEncounterDefinitions())
        encounter->OnTick(botAI);
}

void RaidUlduarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    UldFlameLeviathanDefinition().AddTriggerNodes(triggers);

    UldRazorscaleDefinition().AddTriggerNodes(triggers);

    UldIgnisDefinition().AddTriggerNodes(triggers);

    UldXT002Definition().AddTriggerNodes(triggers);

    UldIronAssemblyDefinition().AddTriggerNodes(triggers);

    UldKologarnDefinition().AddTriggerNodes(triggers);

    UldAuriayaDefinition().AddTriggerNodes(triggers);

    UldHodirDefinition().AddTriggerNodes(triggers);

    UldFreyaDefinition().AddTriggerNodes(triggers);

    UldThorimDefinition().AddTriggerNodes(triggers);

    UldMimironDefinition().AddTriggerNodes(triggers);

    UldVezaxDefinition().AddTriggerNodes(triggers);

    UldYoggSaronDefinition().AddTriggerNodes(triggers);

    UldAlgalonDefinition().AddTriggerNodes(triggers);
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    UldAlgalonDefinition().AddMultipliers(botAI, multipliers);

    UldXT002Definition().AddMultipliers(botAI, multipliers);

    UldMimironDefinition().AddMultipliers(botAI, multipliers);

    UldIronAssemblyDefinition().AddMultipliers(botAI, multipliers);

    UldThorimDefinition().AddMultipliers(botAI, multipliers);

    // Hold the class-generic threat redirects on the bosses where the main tank is the wrong sink
    multipliers.push_back(new UldThreatRedirectMultiplier(botAI));

    // Hold the burst cooldowns on the bosses whose DPS check is not the pull
    multipliers.push_back(new UlduarBurstWindowMultiplier(botAI));

    UldRazorscaleDefinition().AddMultipliers(botAI, multipliers);

    UldIgnisDefinition().AddMultipliers(botAI, multipliers);

    UldKologarnDefinition().AddMultipliers(botAI, multipliers);

    UldFreyaDefinition().AddMultipliers(botAI, multipliers);

    UldFlameLeviathanDefinition().AddMultipliers(botAI, multipliers);

    UldVezaxDefinition().AddMultipliers(botAI, multipliers);

    UldAuriayaDefinition().AddMultipliers(botAI, multipliers);
    UldHodirDefinition().AddMultipliers(botAI, multipliers);

    UldYoggSaronDefinition().AddMultipliers(botAI, multipliers);
}
