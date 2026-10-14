/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALVALUECONTEXT_H
#define PLAYERBOTS_HYJALVALUECONTEXT_H

#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "NamedObjectContext.h"
#include "Value.h"
#include <string>
#include <vector>

class HyjalInfernalsValue : public CalculatedValue<GuidVector>
{
public:
    HyjalInfernalsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
              botAI, "hyjal infernals", HyjalHelpers::INFERNAL_CACHE_INTERVAL) {}

protected:
    GuidVector Calculate() override { return HyjalHelpers::FindInfernalGuids(bot); }
};

// A latch so the gap between MARK_DANGER_MANA and MARK_REJOIN_MANA does not have bots running
// back and forth to/from the group.
class KazrogalBelowManaThresholdValue : public ManualSetValue<bool>
{
public:
    KazrogalBelowManaThresholdValue(PlayerbotAI* botAI)
        : ManualSetValue<bool>(botAI, false, "kaz'rogal below mana threshold") {}
};

class HyjalHazardPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    HyjalHazardPositionsValue(
        PlayerbotAI* botAI, std::string const& name, uint32 spellId, float searchRadius)
        : CalculatedValue<std::vector<Position>>(
              botAI, name, HyjalHelpers::HAZARD_CACHE_INTERVAL),
          _spellId(spellId), _searchRadius(searchRadius) {}

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(bot, _searchRadius, _spellId);
    }

private:
    uint32 const _spellId;
    float const _searchRadius;
};

class RaidHyjalValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidHyjalValueContext()
    {
        creators["hyjal infernals"] = &RaidHyjalValueContext::hyjal_infernals;
        creators["hyjal death and decay"] = &RaidHyjalValueContext::hyjal_death_and_decay;
        creators["hyjal rain of fire"] = &RaidHyjalValueContext::hyjal_rain_of_fire;
        creators["hyjal doomfire trail"] = &RaidHyjalValueContext::hyjal_doomfire_trail;
        creators["kaz'rogal below mana threshold"] =
            &RaidHyjalValueContext::kazrogal_below_mana_threshold;
    }

private:
    static UntypedValue* hyjal_infernals(PlayerbotAI* botAI) {
        return new HyjalInfernalsValue(botAI);
    }
    static UntypedValue* kazrogal_below_mana_threshold(PlayerbotAI* botAI) {
        return new KazrogalBelowManaThresholdValue(botAI);
    }
    static UntypedValue* hyjal_death_and_decay(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal death and decay",
            HyjalHelpers::Id(HyjalHelpers::HyjalSpells::SPELL_DEATH_AND_DECAY),
            HyjalHelpers::DEATH_AND_DECAY_SEARCH_RADIUS);
    }
    static UntypedValue* hyjal_rain_of_fire(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal rain of fire",
            HyjalHelpers::Id(HyjalHelpers::HyjalSpells::SPELL_RAIN_OF_FIRE),
            HyjalHelpers::RAIN_OF_FIRE_SEARCH_RADIUS);
    }
    static UntypedValue* hyjal_doomfire_trail(PlayerbotAI* botAI) {
        return new HyjalHazardPositionsValue(
            botAI, "hyjal doomfire trail",
            HyjalHelpers::Id(HyjalHelpers::HyjalSpells::SPELL_DOOMFIRE_TRAIL),
            HyjalHelpers::DOOMFIRE_SEARCH_RADIUS);
    }
};

#endif
