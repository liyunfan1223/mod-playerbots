/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BurstCooldowns.h"

#include <string>
#include <unordered_set>

#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Timer.h"
#include "Unit.h"

namespace
{
    // CastSpellAction names its action after the spell, so these match the class strategies' NextAction names
    std::unordered_set<std::string> const burstCooldownNames = {
        // raid-wide, racial and item
        "bloodlust", "heroism", "berserking", "blood fury", "use trinket",
        // warrior
        "recklessness", "death wish",
        // rogue
        "adrenaline rush", "blade flurry", "killing spree",
        // mage
        "arcane power", "icy veins", "combustion", "mirror image", "presence of mind",
        // warlock
        "metamorphosis",
        // hunter
        "rapid fire", "bestial wrath", "readiness",
        // priest
        "shadowfiend", "power infusion",
        // druid
        "berserk", "force of nature",
        // paladin
        "avenging wrath",
        // death knight
        "killing machine", "army of the dead", "summon gargoyle",
        // shaman
        "fire elemental totem", "elemental mastery"};
}  // namespace

bool IsBurstCooldownAction(std::string const& actionName)
{
    return burstCooldownNames.find(actionName) != burstCooldownNames.end();
}

bool IsManaReturnCooldown(Player* bot, std::string const& actionName)
{
    return bot && bot->getClass() == CLASS_PRIEST && actionName == "shadowfiend";
}

bool TankHasHeldBoss(Player* bot, Unit* boss, BurstHoldState& state, uint32 dwellMs)
{
    if (!bot || !boss)
    {
        state.Reset();
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.Reset();
        return false;
    }

    Unit* victim = boss->GetVictim();
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    if (!holder || holder->GetGroup() != group || !PlayerbotAI::IsTank(holder))
    {
        state.Reset();
        return false;
    }

    uint32 now = getMSTime();
    if (state.boss != boss->GetGUID() || !state.sinceMs)
    {
        state.boss = boss->GetGUID();
        state.sinceMs = now;
    }

    return getMSTimeDiff(state.sinceMs, now) >= dwellMs;
}
