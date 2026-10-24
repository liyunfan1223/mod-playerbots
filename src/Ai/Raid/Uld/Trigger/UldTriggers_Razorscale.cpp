/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UldTriggers_Razorscale.h"

#include "GameObject.h"
#include "Object.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UldData.h"
#include "UldEncounter_Razorscale.h"
#include "UldScripts.h"
#include "EncounterHelpers.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Trigger.h"
#include "Vehicle.h"
#include <MovementActions.h>
#include <FollowMasterStrategy.h>
#include <RtiTargetValue.h>

using namespace EncounterHelpers;

bool RazorscaleFlyingAloneTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    if (boss->GetPositionZ() < RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return false;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    if (attackers.empty())
        return true;

    std::vector<Unit*> dark_rune_adds;

    for (ObjectGuid const& guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        uint32 entry = unit->GetEntry();

        if (entry == RazorscaleBossHelper::UNIT_DARK_RUNE_WATCHER ||
            entry == RazorscaleBossHelper::UNIT_DARK_RUNE_GUARDIAN ||
            entry == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
            dark_rune_adds.push_back(unit);
    }

    return dark_rune_adds.empty();
}

bool RazorscaleDevouringFlamesTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DEVOURING_FLAME)
            return true;
    }

    return false;
}

bool RazorscaleAvoidSentinelTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL)
            return true;
    }

    return false;
}

bool RazorscaleAvoidWhirlwindTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    for (ObjectGuid const& npc : GetRazorscaleScan(botAI).Hostiles())
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == RazorscaleBossHelper::UNIT_DARK_RUNE_SENTINEL &&
            (unit->HasAura(RazorscaleBossHelper::SPELL_SENTINEL_WHIRLWIND) ||
             unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL)))
            return true;
    }

    return false;
}

bool RazorscaleGroundedTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss)
        return false;

    return boss->GetPositionZ() < RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD;
}

bool RazorscaleHarpoonAvailableTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    RazorscaleBossHelper razorscaleHelper(botAI);

    if (!razorscaleHelper.UpdateBossAI())
        return false;

    // After UpdateBossAI(), which assigns the tank roles as a side effect. Nobody else fires one, so
    // nobody else needs the harpoon search.
    if (!IsRazorscaleHarpoonCrew(bot))
        return false;

    return GetRazorscaleClosestReadyHarpoon(botAI) != nullptr;
}

bool RazorscaleFuseArmorTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !PlayerbotAI::IsMainTank(member))
            continue;

        Aura* fuseArmor = member->GetAura(RazorscaleBossHelper::SPELL_FUSE_ARMOR);
        if (fuseArmor && fuseArmor->GetStackAmount() >= RazorscaleBossHelper::FUSEARMOR_THRESHOLD)
            return true;
    }

    return false;
}

bool RazorscaleKillTargetTrigger::IsActive()
{
    // One bot drives the marking to avoid the whole raid fighting over the icon
    if (!IsMechanicTrackerBot(bot, ULDUAR_MAP_ID))
        return false;

    Unit* target = GetRazorscaleKillTarget(botAI);
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    if (group && group->GetTargetIcon(RtiTargetValue::skullIndex) == target->GetGUID())
        return false;

    return true;
}

bool RazorscalePetControlTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    return bot->GetGuardianPet() != nullptr;
}

bool RazorscaleFlameBreathTrigger::IsActive()
{
    Unit* boss = GetRazorscaleScan(botAI).Boss();
    if (!boss || !boss->IsAlive())
        return false;

    // Flame Breath is a grounded-phase frontal cone; while flying she is not breathing
    if (boss->GetPositionZ() >= RazorscaleBossHelper::RAZORSCALE_FLYING_Z_THRESHOLD)
        return false;

    // The tank holds the boss and eats the breath; only reposition non-tanks caught in the cone
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0))
        return false;

    return IsBotInFrontalCone(bot, boss, RazorscaleBossHelper::FLAME_BREATH_CONE,
                              RazorscaleBossHelper::FLAME_BREATH_RANGE);
}
