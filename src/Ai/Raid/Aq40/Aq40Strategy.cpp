/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Strategy.h"
#include "Aq40Actions.h"
#include "GameTime.h"
#include "Playerbots.h"

namespace
{
// Out of combat, just above follow (1.0) and below everything else, so summoning the Voidwalker,
// buffing and drinking still happen at the start spot.
constexpr float ACTION_PREPULL = ACTION_BG + 1.0f;
}  // namespace

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Arcane Explosion must be handled before Skeram platform travel or target pickup.
    triggers.push_back(new TriggerNode(
        "aq40 encounter",
        {NextAction("aq40 safety", ACTION_EMERGENCY + 2), NextAction("aq40 skeram interrupt", ACTION_RAID + 3),
         NextAction("aq40 tactics", ACTION_RAID + 2), NextAction("aq40 control", ACTION_RAID + 1),
         NextAction("aq40 position", ACTION_LIGHT_HEAL - 1)}));
    // Out of combat: marked Twin Emperors warlocks take their start spots.
    triggers.push_back(new TriggerNode("aq40 twins prepull", {NextAction("aq40 twins prepull", ACTION_PREPULL)}));
}

void RaidAq40Strategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new Aq40Multiplier(botAI));
}

void RaidAq40Strategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::Dps)
        return;
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    uint32 const now = GameTime::GetGameTimeMS().count();
    if (control != _exclusionsControl || control->Version() != _exclusionsVersion || now != _exclusionsTime)
    {
        _exclusionsControl = control;
        _exclusionsVersion = control->Version();
        _exclusionsTime = now;
        _exclusions.clear();
        if (control->Encounter() != Aq40::Aq40Encounter::None)
        {
            Unit* target = control->Target(botAI->GetBot());
            for (Creature* unit : control->Units())
                if (unit != target || !control->AllowedDamage(botAI->GetBot(), unit))
                    _exclusions.insert(unit->GetGUID());
            for (Player* player : control->Members())
                if (player->HasAura(Aq40::Id(Aq40::Aq40Spells::SPELL_TRUE_FULFILLMENT)))
                    _exclusions.insert(player->GetGUID());
        }
    }
    exclusions.insert(_exclusions.begin(), _exclusions.end());
}
